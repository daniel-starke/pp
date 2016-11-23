/**
 * @file Script.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-01-24
 * @version 2016-11-17
 * @remarks May fail to compile with GCC due to a bug for MinGW target within the GC
 * ( https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66488 ).
 */
#include <fstream>
#include <sstream>
#include <boost/assign/list_of.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/scope_exit.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/file/Stream.hpp>
#include <pcf/parser/spirit/GetInfoString.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/string/Convert.hpp>
#include <pcf/string/Escape.hpp>
#include <libpcf/fdio.h>
#include "Script.hpp"
#include "parser/Script.hpp"


namespace pp {


/**
 * Prefix of pre-defined variables.
 */
const std::string Script::preDefPrefix = std::string("PP_");

/**
 * Error location for pre-defined variables.
 */
const LineInfo Script::preDefLocation = LineInfo(boost::filesystem::path("pre-defined variable"));


/**
 * Resets the parsed script information.
 *
 * @remarks The list of imported files is not reset.
 */
void Script::reset() {
	const DynamicVariableSet dynVars = this->vars.getDynamicVariables();
	this->mainSource = boost::filesystem::path();
	this->config = this->initialConfig;
	this->vars = VariableHandler();
	if ( this->config.environmentVariables ) {
		this->vars.addScope(this->environment);
	} else {
		/* this will be filled with the environment variables from the parser
		 * if requested by pragma
		 */
		this->vars.addScope();
	}
	this->vars.addScope();
	this->processes.clear();
	this->targets.clear();
	this->vars.setDynamicVariables(dynVars);
}


/**
 * Reads a given script file.
 *
 * @param[in] path - path to script file
 * @return true on success, else false
 * @throws pcf::exception::FileNotFound if the script file was not found.
 * @throws pcf::exception::Input if it was not possible to read from script file.
 * @throws pcf::exception::SyntaxError if the script file has a syntax error.
 */
bool Script::read(const boost::filesystem::path & path) {
	this->reset();
	this->mainSource = path;
	this->currentSource = path;
	if ( this->readImport(path) ) {
		boost::optional<StringLiteral &> literal;
		/* successfully parsed main script file */
		literal = this->vars.get("progress");
		if (literal && literal->isSet() && ( ! literal->isVariable() )) {
			this->setProgressOutput(boost::filesystem::path(literal->getString(), pcf::path::utf8));
		}
		literal = this->vars.get("progressFormat");
		if (literal && literal->isSet() && ( ! literal->isVariable() )) {
			this->setProgressFormat(literal->getString());
		}
		return true;
	}
	return false;
}


/**
 * Reads a given script file as import. No script is imported more than once.
 * 
 * @param[in] path - path to script file
 * @param[in] sourceFile - main script file
 * @return true on success, else false
 * @throws pcf::exception::FileNotFound if the script file was not found.
 * @throws pcf::exception::Input if it was not possible to read from script file.
 * @throws pcf::exception::SyntaxError if the script file has a syntax error.
 */
bool Script::readImport(const boost::filesystem::path & path, const boost::filesystem::path & sourceFile) {
	const boost::filesystem::path completePath(pcf::path::normalize(boost::filesystem::system_complete(path)));
	if (this->imports.find(completePath) != this->imports.end()) return true; /* imports are only included once */
	const bool result = this->readInclude(path, sourceFile);
	if ( result ) {
		this->imports.insert(completePath);
	}
	return result;
}


/**
 * Reads a given script file as include. The same file can included more than once.
 * 
 * @param[in] path - path to script file
 * @param[in] sourceFile - main script file
 * @return true on success, else false
 * @throws pcf::exception::FileNotFound if the script file was not found.
 * @throws pcf::exception::Input if it was not possible to read from script file.
 * @throws pcf::exception::SyntaxError if the script file has a syntax error.
 */
bool Script::readInclude(const boost::filesystem::path & path, const boost::filesystem::path & sourceFile) {
	const std::string pathStr(path.string(pcf::path::utf8));
	if ( ! (boost::filesystem::exists(path) || pathStr == "-") ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::FileNotFound()
			<< pcf::exception::tag::Message(std::string("Invalid file path \"") + path.string(pcf::path::utf8) + "\".")
		);
	}
	boost::filesystem::path realPath = path;
	if ( ! sourceFile.empty() ) {
		if ( ! path.has_root_path() ) {
			/* is relative path from sourceFile */
			if ( sourceFile.has_parent_path() ) {
				realPath = sourceFile.parent_path() / path;
			}
		}
	}
	boost::scoped_ptr<pcf::file::ifstream> fin;
	std::istream * in;
	if (pathStr == "-") {
		in = &std::cin;
		fdio_setMode(stdin, FDIO_BINARY);
		in->unsetf(std::ios::skipws);
	} else {
		try {
			fin.reset(new pcf::file::ifstream(realPath, std::ifstream::in | std::ifstream::binary));
			fin->unsetf(std::ios::skipws);
			in = static_cast<std::istream *>(fin.get());
		} catch (...) {
			/* failed to open the file */
			BOOST_THROW_EXCEPTION(
				pcf::exception::Input()
				<< pcf::exception::tag::Message(std::string("Failed to open \"") + path.string(pcf::path::utf8) + "\".")
			);
		}
	}
	
	/* input iterator as forward iterator, usable by spirit parser */
	typedef boost::spirit::istream_iterator IteratorType;
	IteratorType begin(*in);
	IteratorType end;
	
	/* wrap forward iterator with position iterator, to record the position */
	typedef boost::spirit::classic::position_iterator2<IteratorType> PosIteratorType;
	PosIteratorType posBegin(begin, end, realPath.string(pcf::path::utf8));
	PosIteratorType posEnd;
	posBegin.set_tab_chars(1);
	posEnd.set_tab_chars(1);
	
	typedef parser::ScriptSkipParser<PosIteratorType> SkipGrammar;
	SkipGrammar skipParser;
	
	typedef parser::Script<PosIteratorType, SkipGrammar> ScriptGrammar;
	ScriptGrammar scriptParser(*this, path);
	
	try {
		const boost::filesystem::path parentSource(this->currentSource);
		this->currentSource = path;
		if ( this->config.environmentVariables ) {
			boost::optional<StringLiteral &> scriptVar = this->vars.get(preDefPrefix + "SCRIPT");
			if ( scriptVar ) {
				scriptVar->setRawString(boost::filesystem::system_complete(path).string(pcf::path::utf8));
			} else {
				this->vars.set(
					preDefPrefix + "SCRIPT",
					StringLiteral(boost::filesystem::system_complete(path).string(pcf::path::utf8), preDefLocation, StringLiteral::RAW)
				);
			}
		}
		BOOST_SCOPE_EXIT(this_, parentSource) {
			if ( this_->config.environmentVariables ) {
				boost::optional<StringLiteral &> scriptVar = this_->vars.get(preDefPrefix + "SCRIPT");
				if ( scriptVar ) {
					scriptVar->setRawString(boost::filesystem::system_complete(parentSource).string(pcf::path::utf8));
				}
			}
			this_->currentSource = parentSource;
		} BOOST_SCOPE_EXIT_END
		if (boost::spirit::qi::phrase_parse(
				posBegin,
				posEnd,
				scriptParser,
				skipParser,
				*this
			) == false) {
			const boost::spirit::classic::file_position_base<std::string> & pos = posBegin.get_position();
			std::ostringstream sout;
			sout << pos.file << ":" << pos.line << ":" << pos.column << ": Failed to parse script file.";
			BOOST_THROW_EXCEPTION(
				pcf::exception::SyntaxError()
				<< pcf::exception::tag::Message(sout.str())
			);
			return false;
		}
	} catch (const boost::spirit::qi::expectation_failure<PosIteratorType> & e) {
		using pcf::parser::spirit::getInnerInfoString;
		const boost::spirit::classic::file_position_base<std::string> & pos = e.first.get_position();
		std::ostringstream sout;
		std::string offset, currentLine;
		currentLine = e.first.get_currentline();
		offset.reserve(static_cast<size_t>(pos.column - 1));
		for (size_t i = 0; i < static_cast<size_t>(pos.column - 1) && i < currentLine.size(); i++) {
			if (currentLine[i] == '\t') {
				offset.push_back('\t');
			} else {
				offset.push_back(' ');
			}
		}
		sout << "Failed to parse script file." << std::endl;
		sout << pos.file << ":" << pos.line << ":" << pos.column << ":" << std::endl
		     << currentLine << std::endl
		     << offset << "^- expected '" << getInnerInfoString(e.what_) << "' here" << std::endl;
		BOOST_THROW_EXCEPTION(
			pcf::exception::SyntaxError()
			<< pcf::exception::tag::Message(sout.str())
		);
		return false;
	}
	return true;
}


/**
 * Prepares the script for execution.
 *
 * @param[in] target - target to execute
 * @return true on success, else false
 */
bool Script::prepare(const std::string & target) {
	using namespace boost::phoenix::placeholders;
	ExecutionMap::iterator aTarget = this->targets.find(target);
	if (aTarget == this->targets.end()) {
		std::cerr << "Error: The target \"" << target << "\" was not defined." << std::endl;
		std::cerr << "Possible targets: ";
		bool isFirst = true;
		BOOST_FOREACH(const ExecutionMap::value_type t, this->targets) {
			if ( ! isFirst ) {
				std::cerr << ", ";
			}
			std::cerr << t.first;
			isFirst = false;
		}
		std::cerr << std::endl;
		return false;
	}
	this->progress.reset();
	if ( ! aTarget->second.prepare(boost::phoenix::bind(&Script::progressUpdate, this, _1, _2)) ) {
		return false;
	}
	this->progressDateTime = boost::posix_time::microsec_clock::universal_time();
	this->progressCount = 0;
	this->progress.start(0);
	return true;
}


/**
 * Executes the previously read and prepared script on the given I/O service.
 *
 * @param[in] target - target to execute
 * @param[in,out] ioService - dispatch work on this I/O service
 * @return true on success, else false
 */
bool Script::execute(const std::string & target, boost::asio::io_service & ioService) {
	using namespace boost::phoenix::placeholders;
	ExecutionMap::iterator aTarget = this->targets.find(target);
	if (aTarget == this->targets.end()) {
		std::cerr << "Error: The target \"" << target << "\" was not defined." << std::endl;
		std::cerr << "Possible targets: ";
		bool isFirst = true;
		BOOST_FOREACH(const ExecutionMap::value_type t, this->targets) {
			if ( ! isFirst ) {
				std::cerr << ", ";
			}
			std::cerr << t.first;
			isFirst = false;
		}
		std::cerr << std::endl;
		return false;
	}
	if (( ! this->config.printOnly ) && this->config.verbosity >= VERBOSITY_INFO) {
		std::cout << "Executing target \"" << target << "\"." << std::endl;
	}
	aTarget->second.execute(
		ioService,
		boost::phoenix::bind(&Script::progressUpdate, this, _1, _2),
		boost::phoenix::bind(&Script::finishedTarget, this, target)
	);
	return true;
}


/**
 * To be called after Script::prepare() and Script::execute(). Finalizes the script
 * execution and write out the logging output for the given target.
 * 
 * @param[in] target - target to finalize
 * @param[in,out] isFirst - set to true before calling this function to handle output formatting
 * correctly
 * @return true on success, else false
 */
bool Script::complete(const std::string & target, bool & isFirst) {
	ExecutionMap::iterator aTarget = this->targets.find(target);
	if (aTarget == this->targets.end()) {
		return false;
	}
	return aTarget->second.complete(isFirst);
}


/**
 * Returns the currently configured verbosity level.
 *
 * @return verbosity level
 */
Verbosity Script::getVerbosity() const {
	return this->config.verbosity;
}


/**
 * Removes the progress output target.
 *
 * @return reference to this object for chained operations
 */
Script & Script::setProgressOutput() {
	this->progressFile.reset();
	this->progressOutput.reset();
	return *this;
}


/**
 * Sets the progress output to a given output stream. The output stream object needs to live
 * longer than this object.
 *
 * @param[in,out] output - write progress output to this stream
 * @return reference to this object for chained operations
 */
Script & Script::setProgressOutput(std::ostream & output) {
	this->progressFile.reset();
	this->progressOutput.reset(output);
	return *this;
}


/**
 * Sets the progress output to a given file.@n
 * Special output files can be defined:@n
 * - stdout = standard output console
 * - stderr = standard error console
 *
 * @param[in,out] output - write progress output to this file
 * @return reference to this object for chained operations
 */
Script & Script::setProgressOutput(const boost::filesystem::path & output) {
	const std::string str(output.string(pcf::path::utf8));
	if (str == "stdout") {
		this->setProgressOutput(std::cout);
	} else if (str == "stderr") {
		this->setProgressOutput(std::cerr);
	} else {
		try {
			this->progressFile.reset(new pcf::file::ofstream(output, std::ofstream::trunc | std::ofstream::out));
			this->progressOutput.reset(*(this->progressFile));
		} catch (...) {
			/* failed to open the file */
			BOOST_THROW_EXCEPTION(
				pcf::exception::Output()
				<< pcf::exception::tag::Message(std::string("Failed to create progress output file at \"") + output.string(pcf::path::utf8) + "\".")
			);
		}
	}
	return *this;
}


/**
 * Sets the progress format accordingly to the given format string.
 * 
 * @param[in] format - format string for progress
 * @return reference to this object for chained operations
 * @see pcf::time::ProgressClock
 */
Script & Script::setProgressFormat(const std::string & format) {
	this->progressFormat = format;
	/* try out format to catch format errors early */
	this->progress.format(this->progressFormat.c_str(), "command", "commands");
	return *this;
}


/**
 * Callback function called to update the current progress.
 *
 * @param[in] addToCurrent - true if the total counter shall be updated, false to update the current
 * state
 * @param[in] commands - number of commands processed
 */
void Script::progressUpdate(const bool addToCurrent, const boost::uint64_t commands) {
	boost::mutex::scoped_lock lock(this->mutex);
	const boost::posix_time::ptime dateTime = boost::posix_time::microsec_clock::universal_time();
	if ( ! addToCurrent ) {
		/* count total */
		this->progress.addTotal(commands);
	} else {
		this->progressCount += commands;
		/* update every second at most */
		if ((dateTime - this->progressDateTime) >= boost::posix_time::seconds(1)
			|| (this->progress.getCurrent() + this->progressCount) >= this->progress.getTotal()) {
			this->progress.add(this->progressCount, dateTime);
			if ( progressOutput ) {
				(*progressOutput) << this->progress.format(this->progressFormat.c_str(), "command", "commands");
				progressOutput->flush();
			}
			this->progressCount = 0;
			this->progressDateTime = dateTime;
		}
	}
}


/**
 * Callback function called after a target was completely executed.
 *
 * @param[in] target - name of the executed target
 */
void Script::finishedTarget(const std::string & target) const {
	if (this->config.verbosity >= VERBOSITY_INFO) std::cout << "Finished execution of target \"" << target << "\"." << std::endl;
}


} /* namespace pp */
