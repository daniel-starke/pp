/**
 * @file Script.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-01-24
 * @version 2016-10-29
 *
 * Data hierarchy:@n
 * - Execution
 * - ProcessNode
 * - ProcessElement
 * - Process
 * - ProcessTransition
 * - ProcessBlock
 * - Command
 * 
 * Call tree:@n
 * - Script::read()
 * - Script::prepare()
 *   - Execution::prepare()
 *     - ProcessNode::traverseDependencies() -> Execution::solveDependencies()
 *     - ProcessNode::traverseBottomUp()     -> Execution::createFlatDependentMap()
 *     - ProcessNode::traverseBottomUp()     -> Execution::checkDuplicates()
 *     - ProcessNode::traverseBottomUp()     -> Execution::createTemporaryInputFileInfoMap()
 *     - ProcessNode::traverseTopDown()      -> Execution::createTemporaryOutputFileInfoMap()
 *     - updateTemporaryCreationFlags()
 *     - ProcessNode::traverseBottomUp()     -> Execution::propagateForcedFlag()
 *     - ProcessNode::traverseBottomUp()     -> Execution::countCommands()
 * - Script::execute()
 *   - Execution::execute()
 *     - ProcessNode::traverseBottomUp()     -> Execution::printCallback()
 *     - ProcessNode::executeChain()         -> Execution::executeProcess() -> Execution::finished() -> Script::finishedTarget()
 * - Script::complete()
 *   - Execution::complete()
 *     - ProcessNode::traverseBottomUp()     -> Execution::logCallback()
 *     - ProcessNode::traverseBottomUp()     -> Execution::updateFlatDependentMap()
 *     - delete temporaries
 *     - ProcessNode::traverseBottomUp()     -> Execution::setFlagsInDatabase()
 */
#ifndef __PP_SCRIPT_HPP__
#define __PP_SCRIPT_HPP__


#include <algorithm>
#include <cstdlib>
#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <pcf/exception/General.hpp>
#include <pcf/file/Stream.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/time/Utility.hpp>
#include "Execution.hpp"
#include "Variable.hpp"
#include "Type.hpp"


namespace pp {


/**
 * Class to handle script parsing and execution.
 */
class Script {
template <typename Iterator, typename Skipper>
friend struct parser::Script;
public:
	static const std::string preDefPrefix;
	static const LineInfo preDefLocation;
private:
	boost::filesystem::path mainSource; /**< Path to the main source file. */
	boost::filesystem::path currentSource; /**< Path to the currently parsing source file. */
	Configuration config; /**< Script configuration. */
	Configuration initialConfig; /**< Initial script configuration. */
	VariableMap environment; /**< Pre-defined environment variables to be passed to the script. */
	VariableHandler vars; /**< Variable handler for variable substitution. */
	ShellMap shells; /**< Used command shell for command execution. */
	ProcessMap processes; /**< List of defined processes mapped by its ID. */
	ExecutionMap targets; /**< List of defined execution targets mapped by its ID. */
	std::set<boost::filesystem::path> imports; /**< List of already imported files. */
	boost::shared_ptr<pcf::file::ofstream> progressFile; /**< Handle to the progress output file. */
	boost::optional<std::ostream &> progressOutput; /**< Handle to the progress output stream. */
	std::string progressFormat; /**< Progress format string. */
	pcf::time::ProgressClock progress; /**< Instance of a progress output generator. */
	boost::posix_time::ptime progressDateTime; /**< Start date time of target execution. */
	boost::uint64_t progressCount; /**< Number of progressed files. */
	mutable boost::mutex mutex; /**< Mutex object for parallel execution. */
public:
	/**
	 * Constructor.
	 *
	 * @param[in] c - initial script configuration
	 * @param[in] vh - initial variable handler which may contain the environment variables in its
	 * global most scope
	 */
	explicit Script(const Configuration & c, const VariableHandler & vh):
		config(c),
		initialConfig(c),
		environment(),
		vars(vh)
	{
		this->vars.addDynamicVariable("?");
		this->vars.addDynamicVariable("*");
		this->vars.addDynamicVariable("@*");
		this->vars.addDynamicVariable("PP_THREAD");
		
		VariableScopes & vs(this->vars.getScopes());
		if ( vs.empty() ) {
			this->vars.addScope();
		} else {
			this->environment = vs.front();
		}

		/* set default shell */
		Shell defaultShell;
		std::string shellCmdLine;
#ifdef PCF_IS_WIN
		boost::optional<const StringLiteral &> usedShell = vh.get("ComSpec");
		if ( usedShell ) {
			defaultShell.path = pcf::path::resolveExecutable(boost::filesystem::path(usedShell->getString(), pcf::path::utf8));
			shellCmdLine = "\"" + usedShell->getString() + "\"";
		} else {
			defaultShell.path = pcf::path::resolveExecutable(boost::filesystem::path(std::string("cmd.exe"), pcf::path::utf8));
			shellCmdLine = std::string("cmd.exe");
		}
		shellCmdLine += " /c {?}";
		defaultShell.raw = true;
#else /* non Windows */
		boost::optional<const StringLiteral &> usedShell = vh.get("SHELL");
		if ( usedShell ) {
			shellCmdLine = "\"" + usedShell->getString() + "\"";
			defaultShell.path = pcf::path::resolveExecutable(boost::filesystem::path(usedShell->getString(), pcf::path::utf8));
		} else {
			shellCmdLine = std::string("sh");
			defaultShell.path = pcf::path::resolveExecutable(boost::filesystem::path(std::string("sh"), pcf::path::utf8));
		}
		shellCmdLine += " -c \"{?}\"";
		defaultShell.addReplacement("/\\\\/\\\\");
		defaultShell.addReplacement("/\"/\\\"");
		defaultShell.raw = false;
#endif /* Windows */
		defaultShell.cmdLine = StringLiteral(
			shellCmdLine,
			pp::LineInfo(),
			pp::StringLiteral::STANDARD | pp::StringLiteral::NO_ESCAPE
		);
		shells["default"] = defaultShell;
	}
	
	void reset();
	bool read(const boost::filesystem::path & path);
	bool prepare(const std::string & target);
	bool execute(const std::string & target, boost::asio::io_service & ioService);
	bool complete(const std::string & target, bool & isFirst);
	
	Verbosity getVerbosity() const;
	
	Script & setProgressOutput();
	Script & setProgressOutput(std::ostream & output);
	Script & setProgressOutput(const boost::filesystem::path & output);
	Script & setProgressFormat(const std::string & format);
private:
	bool readImport(const boost::filesystem::path & path, const boost::filesystem::path & sourceFile = boost::filesystem::path());
	bool readInclude(const boost::filesystem::path & path, const boost::filesystem::path & sourceFile = boost::filesystem::path());
	void progressUpdate(const bool addToCurrent, const boost::uint64_t commands);
	void finishedTarget(const std::string & target) const;
};



} /* namespace pp */


#endif /* __PP_SCRIPT_HPP__ */
