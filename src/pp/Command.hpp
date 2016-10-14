/**
 * @file Command.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-03-22
 * @version 2016-05-01
 */
#ifndef __PP_COMMAND_HPP__
#define __PP_COMMAND_HPP__


#include <cstdlib>
#include <iosfwd>
#include <sstream>
#include <string>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <pcf/exception/General.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/process/Utility.hpp>
#include "Variable.hpp"
#include "Type.hpp"


namespace pp {


/**
 * Class to handle a single execution command.
 *
 * @todo write command output to database
 */
class Command {
template <typename Iterator, typename Skipper>
friend struct parser::Script;
public:
	/** Possible command states. */
	enum State {
		IDLE,     /**< The command was not started yet. */
		RUNNING,  /**< The command is currently running. */
		FINISHED, /**< The command finished successfully. */
		FAILED    /**< An error occurred running the command. */
	};
private:
	Shell shell; /**< Description of the used shell. */
	StringLiteral command; /**< The command to execute. */
	boost::posix_time::ptime startDt; /**< Start date time. */
	boost::posix_time::ptime endDt; /**< End date time. */
	std::string output; /**< Command output. */
	int exitCode; /**< Command exit code. */
	State state; /**< Current command state. @see State */
	mutable boost::mutex mutex; /**< Command mutex. */
public:
	/** Default constructor. */
	explicit Command() {
		this->reset();
	}
	
	/**
	 * Constructor.
	 * 
	 * @param[in] sh - description of the desired execution shell
	 * @param[in] cmd - command to execute
	 */
	explicit Command(const Shell & sh, const StringLiteral & cmd):
		shell(sh),
		command(cmd),
		state(IDLE)
	{}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - instance to copy
	 */
	Command(const Command & o):
		shell(o.shell),
		command(o.command),
		startDt(o.startDt),
		endDt(o.endDt),
		output(o.output),
		exitCode(o.exitCode),
		state(o.state)
	{}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - instance to assign
	 * @return reference to this object for chained operations
	 */
	Command & operator= (const Command & o) {
		if (this != &o) {
			this->shell = o.shell;
			this->command = o.command;
			this->startDt = o.startDt;
			this->endDt = o.endDt;
			this->output = o.output;
			this->exitCode = o.exitCode;
			this->state = o.state;
		}
		return *this;
	}
	
	/**
	 * Replace variables that are referred within the internal command
	 * with available variables from the given variable handler.
	 *
	 * @param[in] vars - variable handle with variables for replacement
	 */
	void replaceVariables(const VariableHandler & vars) {
		this->command.replaceVariables(vars);
	}
	
	/**
	 * Returns a reference to the contained command which shall be executed.
	 *
	 * @return command as string literal
	 */
	const StringLiteral & getCommandString() const {
		return this->command;
	}
	
	/**
	 * Returns the final command-line by using the internal shell description.
	 *
	 * @return final command-line as string literal
	 */
	StringLiteral getFinalCommandString() const {
		StringLiteral aCommand(this->command);
		StringLiteral result(this->shell.cmdLine);
		VariableMap replacements;
		replacements["?"] = StringLiteral(this->shell.replace(aCommand.getString()), aCommand.getLineInfo(), StringLiteral::RAW);
		result.replaceVariables(replacements);
		return result;
	}
	
	/**
	 * Replaces referred variables of the internally stored command with the ones
	 * defined in the given variable handler and returns the resulting final
	 * command-line build from the internal shell description.
	 *
	 * @param[in] vars - variable handle with variables for replacement
	 * @return final command-line as string literal
	 * @todo add check whether command got completely replaced or not
	 */
	StringLiteral getFinalCommandString(const VariableHandler & vars) const {
		StringLiteral aCommand(this->command);
		aCommand.replaceVariables(vars);
		StringLiteral result(this->shell.cmdLine);
		VariableMap replacements;
		replacements["?"] = StringLiteral(this->shell.replace(aCommand.getString()), aCommand.getLineInfo(), StringLiteral::RAW);
		result.replaceVariables(replacements);
		return result;
	}
	
	/**
	 * Returns the current command state.
	 *
	 * @return current command state
	 */
	State getState() const {
		return this->state;
	}
	
	/**
	 * Returns true if the command completed regardless its result.
	 *
	 * @return true if executed completely, else false
	 */
	bool completed() const {
		switch (this->state) {
		case IDLE:
		case RUNNING:
			return false;
			break;
		default:
			break;
		}
		return true;
	}
	
	/**
	 * Returns the start and end date times.
	 *
	 * @param[out] start - output variable for the start date time
	 * @param[out] end - output variable for the end date time
	 */
	void getTimes(boost::posix_time::ptime & start, boost::posix_time::ptime & end) const {
		start = this->startDt;
		end = this->endDt;
	}
	
	/** 
	 * Returns the command output as raw string.
	 *
	 * @return command output
	 */
	std::string getOutput() const {
		return this->output;
	}
	
	/**
	 * Returns the command exit code.
	 *
	 * @return command exit code
	 */
	int getExitCode() const {
		return this->exitCode;
	}
	
	/**
	 * Resets the internal command states for a new execution.
	 */
	void reset() {
		boost::mutex::scoped_lock lock(this->mutex);
		this->startDt = boost::posix_time::ptime();
		this->endDt = boost::posix_time::ptime();
		this->output.clear();
		this->exitCode = 0;
		this->state = IDLE;
	}
	
	/**
	 * Outputs the command result to the given output stream.
	 *
	 * @param[in,out] out - output stream to write to
	 * @param[in,out] wroteOutput - set to true if output was written, sets nothing else
	 */
	void printResults(std::ostream & out, bool & wroteOutput) const {
		if ( this->startDt.is_not_a_date_time() ) {
			/* command was not executed */
			out << "\nCommand was not executed: " << this->getFinalCommandString().getString() << '\n';
			wroteOutput = true;
			return;
		}
		out << '\n' << Command::dateTimeToString(this->startDt) << ": " <<  this->getFinalCommandString().getString() << '\n';
		wroteOutput = true;
		std::string newOutput;
		char lastChar = '\n';
		newOutput.reserve(this->output.size());
		BOOST_FOREACH(const char c, this->output) {
			if (c != '\r') {
				newOutput.push_back(c);
				lastChar = c;
			}
		}
		if ( ! boost::algorithm::all_of(newOutput.begin(), newOutput.end(), boost::algorithm::is_any_of(" \t\n\r")) ) {
			out.write(newOutput.c_str(), static_cast<std::streamsize>(newOutput.size()));
		}
		if (lastChar != '\n') out << '\n';
		if (this->exitCode == 0) {
			out
				<< Command::dateTimeToString(this->endDt)
				<< ": Command finished successfully after "
				<< (this->endDt - this->startDt).total_seconds()
				<< " seconds.\n";
		} else {
			out
				<< Command::dateTimeToString(this->endDt)
				<< ": Command exit with exit code "
				<< this->exitCode
				<< " after "
				<< (this->endDt - this->startDt).total_seconds()
				<< " seconds.\n";
		}
	}
	
	/**
	 * Prepares the command for execution. Referred variables within the internal
	 * command string are replaced by known variables in the given variable handler.
	 * The passed configuration decides how to tread missing variable substitutions.
	 *
	 * @param[in] vars - variable handle with variables for replacement
	 * @param[in] config - configuration for variable replacement
	 * @return true on success, else false
	 * @see Configuration::variableChecking
	 */
	bool prepare(const VariableHandler & vars, const Configuration & config) {
		std::string unknownVariable;
		this->reset();
		boost::mutex::scoped_lock lock(this->mutex);
		if ( ! this->command.replaceVariables(unknownVariable, vars) ) {
			if ( config.variableChecking ) {
				BOOST_THROW_EXCEPTION(
					pcf::exception::SymbolUnknown()
					<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(this->command.getLineInfo()) + ": Error: Trying to access unknown variable \"" + unknownVariable + "\".")
				);
				return false;
			} else {
				if (config.verbosity >= VERBOSITY_WARN) {
					std::cerr << this->command.getLineInfo() << " Warning: Trying to access unknown variable \"" << unknownVariable << "\"." << std::endl;
				}
			}
		}
		return true;
	}
	
	/**
	 * Executes the internal command within the current thread.
	 *
	 * @param[in] checkCommand - sets the internal state to FAILED if the command returns
	 * an exit code different from 0 (disabled by default)
	 * @return true on success (state == FINISHED), else false
	 * @remarks the command is blocked for subsequently executions until it has finished execution
	 * @todo read command output as UTF-8 if configured in that way
	 */
	bool execute(const bool checkCommand = false) {
		boost::mutex::scoped_lock lock(this->mutex);
		this->state = RUNNING;
		/* execute and fill class attributes */
		boost::optional<pcf::process::ProcessPipe> proc;
		std::ostringstream sout;
		this->startDt = boost::posix_time::microsec_clock::universal_time();
		this->endDt = boost::posix_time::ptime();
#ifdef PCF_IS_WIN
		const std::wstring shellPath(this->shell.path.wstring(pcf::path::utf8));
		const std::wstring cmd(boost::locale::conv::utf_to_utf<wchar_t>(this->getFinalCommandString().getString()));
#else /* not Windows */
		const std::string shellPath(this->shell.path.string(pcf::path::utf8));
		const std::string cmd(this->getFinalCommandString().getString());
#endif /* Windows */
#ifdef PCF_IS_WIN
		if ( this->shell.raw ) {
			proc = boost::optional<pcf::process::ProcessPipe>(pcf::process::ProcessPipeFactory(
				shellPath,
				cmd,
				pcf::process::ProcessPipeFactory::Type::Raw
			).run());
		} else
#endif /* Windows */
		{
			proc = boost::optional<pcf::process::ProcessPipe>(pcf::process::ProcessPipeFactory(
				shellPath,
				cmd,
				pcf::process::ProcessPipeFactory::Type::Shell
			).run());
		}
		if ( ( ! proc ) || ( ! (*proc) ) ) {
			this->endDt = this->startDt;
			this->exitCode = -1;
			this->state = FAILED;
			return false;
		}
		lock.unlock();
		{
			/* @todo read to UTF-8 on Windows */
			boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_source> in(
				proc->getOutFd(),
				boost::iostreams::never_close_handle
			);
			/* write program output to this->output */
			sout << static_cast<std::streambuf *>(&in);
		}
		lock.lock();
		this->endDt = boost::posix_time::microsec_clock::universal_time();
		this->output = sout.str();
		this->exitCode = proc->wait();
		if (this->exitCode < 0) this->exitCode = 1;
		if (checkCommand && this->exitCode != 0) {
			this->state = FAILED;
		} else {
			this->state = FINISHED;
		}
		return (this->state == FINISHED);
	}
private:
	/**
	 * Internal helper function to convert a date time stamp to string.
	 *
	 * @param[in] dt - date time to convert
	 * @return string representation of the passed date time
	 */
	static std::string dateTimeToString(const boost::posix_time::ptime & dt) {
		if ( dt.is_special() ) return std::string("????""-??""-?? ??:??:??.???");
		const boost::gregorian::date d(dt.date());
		const boost::posix_time::time_duration t(dt.time_of_day());
		return boost::str(boost::format("%04i-%02i-%02i %02i:%02i:%02i.%03i")
			% d.year()
			% static_cast<int>(d.month())
			% d.day()
			% t.hours()
			% t.minutes()
			% t.seconds()
			% (t.total_milliseconds() % 1000)
		);
	}
};


} /* namespace pp */


#endif /* __PP_COMMAND_HPP__ */
