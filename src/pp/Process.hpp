/**
 * @file Process.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-03-22
 * @version 2017-09-16
 */
#ifndef __PP_PROCESS_HPP__
#define __PP_PROCESS_HPP__


#include <cstdlib>
#include <iosfwd>
#include <string>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include <boost/asio.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/make_shared.hpp>
#include <boost/regex.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <pcf/exception/General.hpp>
#include <pcf/file/Stream.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/string/Escape.hpp>
#include "ProcessBlock.hpp"
#include "Variable.hpp"
#include "Type.hpp"


namespace pp {


/**
 * Class to handle a single process which can be made up of any number of process blocks.
 */
class Process {
template <typename Iterator, typename Skipper>
friend struct parser::Script;
public:
	/** Possible process states. */
	enum State {
		IDLE,     /**< The process was not started yet. */
		RUNNING,  /**< The process is currently running. */
		FINISHED, /**< The process finished successfully. */
		FAILED    /**< An error occurred running the process. */
	};
private:
	LineInfo lineInfo; /**< Line information where this process was defined. */
	Configuration config; /**< Configuration to enforce. */
	ProcessBlockVector processBlocks; /**< Process blocks. */
	ProcessTransitionVector transitions; /**< File transitions due to the process block definitions. */
	std::string id; /**< ID of the process. */
	size_t transitionsInQueue; /**< Number of remaining transitions within the execution queue. */
	State state; /**< Current process state. @see State */
	mutable boost::mutex mutex; /**< Mutex object for parallel execution. */
public:
	/** Constructor. */
	explicit Process():
		transitionsInQueue(0),
		state(IDLE)
	{}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	Process(const Process & o):
		lineInfo(o.lineInfo),
		config(o.config),
		processBlocks(o.processBlocks),
		transitions(o.transitions),
		id(o.id),
		transitionsInQueue(o.transitionsInQueue),
		state(o.state)
	{}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 * @return reference to this object for chained operations
	 */
	Process & operator= (const Process & o) {
		if (this != &o) {
			this->lineInfo = o.lineInfo;
			this->config = o.config;
			this->processBlocks = o.processBlocks;
			this->transitions = o.transitions;
			this->id = o.id;
			this->transitionsInQueue = o.transitionsInQueue;
			this->state = o.state;
		}
		return *this;
	}
	
	/**
	 * Include the process blocks of the given process in this process.
	 *
	 * @param[in] o - include process blocks from this process
	 */
	Process & include(const Process & o) {
		if (this != &o) {
			boost::push_back(this->processBlocks, o.processBlocks);
			boost::push_back(this->transitions, o.transitions);
		}
		return *this;
	}
	
	/**
	 * Returns the line information where this process was defined.
	 *
	 * @return defined script location
	 */
	const LineInfo & getLineInfo() const {
		return this->lineInfo;
	}
	
	/**
	 * Sets the line information where this process was defined.
	 * 
	 * @param[in] li - defined script location
	 * @return reference to this object for chained operations
	 */
	Process & setLineInfo(const LineInfo & li) {
		this->lineInfo = li;
		return *this;
	}
	
	/**
	 * Returns the ID of this process.
	 *
	 * @return process ID
	 */
	std::string getId() const {
		return this->id;
	}
	
	/**
	 * Returns the output files which will be created by this process.
	 *
	 * @return list of output files
	 */
	PathLiteralPtrVector getOutputs() {
		PathLiteralPtrVector result;
		BOOST_FOREACH(ProcessTransition & transition, this->transitions) {
			boost::push_back(result, transition.output);
		}
		return result;
	}
	
	/**
	 * Resets the internal process states for a new execution.
	 */
	void reset() {
		boost::mutex::scoped_lock lock(this->mutex);
		this->transitions.clear();
		this->transitionsInQueue = 0;
		this->state = IDLE;
	}
	
	/**
	 * Completes the internal destination variables by resolving remaining variable references
	 * using the passed variable handler.
	 *
	 * @param[in] vars - variable handle with variables for replacement
	 */
	void completeDestinationVariables(const VariableHandler & vars) {
		BOOST_FOREACH(ProcessBlock & processBlock, this->processBlocks) {
			processBlock.completeDestinationVariables(vars);
		}
	}
	
	/**
	 * Completes the internal commands by resolving remaining variable references
	 * using the passed variable handler.
	 *
	 * @param[in] vars - variable handle with variables for replacement
	 */
	void completeCommandVariables(const VariableHandler & vars) {
		BOOST_FOREACH(ProcessBlock & processBlock, this->processBlocks) {
			processBlock.completeCommandVariables(vars);
		}
	}
	
	/**
	 * Creates a list of input files from a given initial input file definition.
	 * 
	 * @param[in] strLit - string with with the initial input file definition
	 * @param[in,out] input - add new input files to this output variable
	 * @return true on success, else false
	 * @remarks The definition can be a regular expression which matches all files on the
	 * file system which shall be included or a path to a file (if the path starts with @)
	 * which contains a file per line as a list of files.
	 *
	 */
	bool createInitialInputList(const StringLiteral & strLit, PathLiteralPtrVector & input) {
		const std::string str(strLit.getString());
		if ( str.empty() ) return true; /* can only happen for process block type "none" */
		if (str[0] != '@') {
			return this->createInitialInputListFromRegEx(str, strLit.getLineInfo(), input);
		}
		return this->createInitialInputListFromFile(str, strLit.getLineInfo(), input);
	}
	
	/**
	 * Creates a list of input files from a given input file.
	 * 
	 * @param[in] inFile - read list of input files from this file (line-wise)
	 * @param[in] li - script location where this input file was defined
	 * @param[in,out] input - add new input files to this output variable
	 * @return true on success, else false
	 * @see createInitialInputList()
	 */
	bool createInitialInputListFromFile(const std::string & inFile, const LineInfo & li, PathLiteralPtrVector & input) const {
		const boost::filesystem::path inputFile(inFile.substr(1), pcf::path::utf8);
		if ( ! (boost::filesystem::exists(inputFile) && boost::filesystem::is_regular_file(inputFile)) ) {
			std::ostringstream sout;
			sout << li << ": Error: Input file list file does not exist \"" << inputFile.string(pcf::path::utf8) + "\".";
			BOOST_THROW_EXCEPTION(
				pcf::exception::FileNotFound()
				<< pcf::exception::tag::Message(sout.str())
			);
			return false;
		}
		pcf::file::ifstream in(inputFile);
		if ( ! in.is_open() ) {
			std::ostringstream sout;
			sout << li << ": Error: Failed to read file list input file \"" << inputFile.string(pcf::path::utf8) + "\".";
			BOOST_THROW_EXCEPTION(
				pcf::exception::Input()
				<< pcf::exception::tag::Message(sout.str())
			);
			return false;
		}
		/* read input files line by line */
		LineInfo pathLineInfo(inputFile);
		size_t lineNr = 1;
		for (std::string line; std::getline(in, line); pathLineInfo.line = ++lineNr) {
			boost::algorithm::trim(line);
			if ( line.empty() ) continue; /* ignore lines with only whitespaces */
			const boost::filesystem::path inPath(line, pcf::path::utf8);
			if ( boost::filesystem::exists(inPath) ) {
				input.push_back(boost::make_shared<PathLiteral>(line, pathLineInfo, StringLiteral::RAW));
				input
					.back()
					->setFlags(PathLiteral::PERMANENT | PathLiteral::EXISTS)
					.setLastModification(boost::posix_time::from_time_t(boost::filesystem::last_write_time(inPath)));
			} else {
				if (this->config.verbosity >= VERBOSITY_WARN) {
					std::cerr << inputFile.string(pcf::path::utf8) << ':' << lineNr
						<< ": Warning: Input file does not exist and will be ignored \""
						<< inPath.string(pcf::path::utf8) << "\"." << std::endl;
				}
			}
		}
		return true;
	}
	
	/**
	 * Creates a list of input files from a given regular expression.
	 * 
	 * @param[in] inRegEx - match files on the file system against this regular expression
	 * @param[in] li - script location where this regular expression was defined
	 * @param[in,out] input - add new input files to this output variable
	 * @return true on success, else false
	 * @see createInitialInputList()
	 */
	bool createInitialInputListFromRegEx(const std::string & inRegEx, const LineInfo & li, PathLiteralPtrVector & input) const {
		std::vector<boost::filesystem::path> pathList;
		const std::wstring inRegExW(boost::locale::conv::utf_to_utf<wchar_t>(reduceConsecutiveSlashes(inRegEx)));
		try {
			boost::wregex regex(
				inRegExW,
				boost::regex_constants::perl
#if defined(PCF_IS_WIN)
				| boost::regex_constants::icase
#endif /* PCF_IS_WIN */
			);
			const RegExNamedCaptureSet namedCaptures = getRegExCaptureNames(inRegEx);
			try {
				pcf::path::getRegexPathList(pathList, inRegExW, this->config.fullRecursiveMatch);
			} catch (const boost::regex_error & e) {
				std::ostringstream sout;
				sout << li << ": Error: Regular expression '" << inRegEx << "' is invalid.\n" << e.what();
				BOOST_THROW_EXCEPTION(
					pcf::exception::SyntaxError()
					<< pcf::exception::tag::Message(sout.str())
				);
				return false;
			} catch (...) {
				std::ostringstream sout;
				sout << li << ": Error: Failed to get path list from regular expression \"" << inRegEx << "\".";
				BOOST_THROW_EXCEPTION(
					pcf::exception::File()
					<< pcf::exception::tag::Message(sout.str())
				);
				return false;
			}
			BOOST_FOREACH(const boost::filesystem::path & p, pathList) {
				const std::string str(pcf::path::correctSeparator(p).string(pcf::path::utf8));
				const std::wstring wstr(pcf::path::correctSeparator(p).wstring(pcf::path::utf8));
				boost::wsmatch what;
				if (boost::regex_match(wstr, what, regex) && ( ! what.empty() )) {
					input.push_back(boost::make_shared<PathLiteral>(str, this->lineInfo, StringLiteral::RAW));
					VariableMap captures;
					for (int i = 0; i < static_cast<int>(what.size()); i++) {
						const boost::wssub_match & match(what[i]);
						if ( match.matched ) {
							captures[boost::lexical_cast<std::string>(i)] = pcf::string::escapeCharacters(std::string(match.first, match.second), '\\', "\\\"");
						}
					}
					BOOST_FOREACH(const std::string & tag, namedCaptures) {
						try {
							const boost::wssub_match & match(what[tag]);
							if ( match.matched ) {
								captures[tag] = pcf::string::escapeCharacters(std::string(match.first, match.second), '\\', "\\\"");
							}
						} catch (...) {
							/* ignore unknown captures */
						}
					}
					input
						.back()
						->setFlags(PathLiteral::PERMANENT | PathLiteral::EXISTS)
						.setLastModification(boost::posix_time::from_time_t(boost::filesystem::last_write_time(p)))
						.setRegexCaptures(captures);
				} else if (this->config.verbosity >= VERBOSITY_WARN) {
					std::cerr << li << ": Warning: Previously found input file ignored after strict mismatch: " << str << std::endl;
					std::cerr << li << ": Hint: Check mismatching regular expression for multiple separators or relative path elements." << std::endl;
				}
			}
		} catch (const boost::regex_error & e) {
			std::ostringstream sout;
			sout << li << ": Error: Regular expression '" << inRegEx << "' is invalid.\n" << e.what();
			BOOST_THROW_EXCEPTION(
				pcf::exception::SyntaxError()
				<< pcf::exception::tag::Message(sout.str())
			);
			return false;
		}
		return true;
	}
	
	/**
	 * Creates all transitions from the list of input files using the internal process block
	 * definitions.
	 *
	 * @param[in] input - list of input files
	 * @return true on success, else false
	 */
	bool createDependencyList(PathLiteralPtrVector & input) {
		bool result = true;
		BOOST_FOREACH(const ProcessBlock & processBlock, this->processBlocks) {
			result = result && processBlock.createTransitions(input, this->transitions, this->config);
		}
		return result;
	}
	
	/**
	 * Create flat dependent map by adding the relations of the internal transitions to the given map.
	 *
	 * @param[in,out] flatDependentMap - update this map
	 */
	void createFlatDependentMap(PathLiteralPtrDependentMap & flatDependentMap) const {
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, transition.output) {
				if (literal->hasFlags(PathLiteral::MODIFIED) || ( ! literal->hasFlags(PathLiteral::EXISTS) )) {
					if ( transition.dependency.empty() ) {
						flatDependentMap.insert(std::make_pair(literal, PathLiteralPtrDependentMap::mapped_type()));
					} else {
						BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & dependent, transition.dependency) {
							const boost::filesystem::path path(dependent->getString(), pcf::path::utf8);
							flatDependentMap[literal].insert(path);
						}
					}
				} else {
					flatDependentMap.insert(std::make_pair(literal, PathLiteralPtrDependentMap::mapped_type()));
				}
			}
		}
	}
	
	/**
	 * Update the given file information map according to the internal transitions for all input files.
	 *
	 * @param[in,out] temporaryFileInfoMap - update this map
	 */
	void createTemporaryInputFileInfoMap(TemporaryFileInfoMap & temporaryFileInfoMap) const {
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			/* handle temporary outputs */
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & output, transition.output) {
				if ( output->hasFlags(PathLiteral::TEMPORARY) ) {
					TemporaryFileInfo & fileInfo(temporaryFileInfoMap[output]);
					BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & input, transition.dependency) {
						if ( ! input->hasFlags(PathLiteral::TEMPORARY) ) {
							/* we only need the first non-temporary input */
							fileInfo.input.insert(input);
							fileInfo.allInputExists = fileInfo.allInputExists && input->hasFlags(PathLiteral::EXISTS);
							fileInfo.inputWasModified = fileInfo.inputWasModified || input->hasFlags(PathLiteral::MODIFIED);
						} else {
							/* get permanent input from already built up map */
							TemporaryFileInfo & temporaryFileInfo(temporaryFileInfoMap[input]);
							fileInfo.input.insert(temporaryFileInfo.input.begin(), temporaryFileInfo.input.end());
							if ( ! temporaryFileInfo.mostRecentInputChange.is_not_a_date_time() ) {
								/* update most recent input file modification date time */
								if (fileInfo.mostRecentInputChange.is_not_a_date_time() || fileInfo.mostRecentInputChange < temporaryFileInfo.mostRecentInputChange) {
									fileInfo.mostRecentInputChange = temporaryFileInfo.mostRecentInputChange;
								}
							}
							fileInfo.allInputExists = fileInfo.allInputExists && temporaryFileInfo.allInputExists;
							fileInfo.inputWasModified = fileInfo.inputWasModified || temporaryFileInfo.inputWasModified;
						}
						if ( ! input->getLastModification().is_not_a_date_time() ) {
							/* update most recent input file modification date time */
							if (fileInfo.mostRecentInputChange.is_not_a_date_time() || fileInfo.mostRecentInputChange < input->getLastModification()) {
								fileInfo.mostRecentInputChange = input->getLastModification();
							}
						}
					}
				}
			}
		}
	}
	
	/**
	 * Update the given file information map according to the internal transitions for all output files.
	 *
	 * @param[in,out] temporaryFileInfoMap - update this map
	 */
	void createTemporaryOutputFileInfoMap(TemporaryFileInfoMap & temporaryFileInfoMap) const {
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			/* handle temporary inputs */
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & input, transition.dependency) {
				if ( input->hasFlags(PathLiteral::TEMPORARY) ) {
					TemporaryFileInfo & fileInfo(temporaryFileInfoMap[input]);
					BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & output, transition.output) {
						if ( ! output->hasFlags(PathLiteral::TEMPORARY) ) {
							/* we only need the first non-temporary output */
							fileInfo.output.insert(output);
							fileInfo.allOutputExists = fileInfo.allOutputExists && output->hasFlags(PathLiteral::EXISTS);
							fileInfo.outputWillBeModified = fileInfo.outputWillBeModified || output->hasFlags(PathLiteral::MODIFIED) || output->hasFlags(PathLiteral::FORCED);
						} else {
							/* get permanent output from already built up map */
							TemporaryFileInfo & temporaryFileInfo(temporaryFileInfoMap[output]);
							fileInfo.output.insert(temporaryFileInfo.output.begin(), temporaryFileInfo.output.end());
							if ( ! temporaryFileInfo.oldestOutputChange.is_not_a_date_time() ) {
								/* update oldest output file modification date time */
								if (fileInfo.oldestOutputChange.is_not_a_date_time() || fileInfo.oldestOutputChange > temporaryFileInfo.oldestOutputChange) {
									fileInfo.oldestOutputChange = temporaryFileInfo.oldestOutputChange;
								}
							}
							fileInfo.allOutputExists = fileInfo.allOutputExists && temporaryFileInfo.allOutputExists;
							fileInfo.outputWillBeModified = fileInfo.outputWillBeModified || temporaryFileInfo.outputWillBeModified;
						}
						if ( ! output->getLastModification().is_not_a_date_time() ) {
							/* update oldest output file modification date time */
							if (fileInfo.oldestOutputChange.is_not_a_date_time() || fileInfo.oldestOutputChange > output->getLastModification()) {
								fileInfo.oldestOutputChange = output->getLastModification();
							}
						}
					}
				}
			}
		}
	}
	
	/**
	 * Propagate the forced build flag to all dependent files.
	 */
	void propagateForcedFlag() const {
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			bool hasForcedInputDependency(false);
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & input, transition.dependency) {
				if ( input->hasFlags(PathLiteral::FORCED) ) {
					hasForcedInputDependency = true;
					break;
				}
			}
			if ( hasForcedInputDependency ) {
				/* propagate single input dependency forced build to all its outputs */
				BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & output, transition.output) {
					output->addFlags(PathLiteral::FORCED);
				}
			}
		}
	}
	
	/**
	 * Returns the number of defined commands (according to the number of input files) which shall
	 * be executed for progress visualization.
	 *
	 * @return number of commands to be executed
	 */
	boost::uint64_t countCommands() const {
		boost::uint64_t result = 0;
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			result += static_cast<boost::uint64_t>(transition.commands.size());
		}
		return result;
	}
	
	/**
	 * Outputs to the command-lines to the given output streams which would be executed.
	 *
	 * @param[in,out] out - output to this output stream
	 * @param[in] prefix - add this prefix to each command
	 */
	void print(std::ostream & out, const std::string & prefix = std::string()) {
		const std::string innerPrefix(prefix + "\t");
		out << prefix << "process : " << this->id << " {\n";
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			int reasonFlags;
			if ( this->transitionNeedsBuild(transition, reasonFlags) ) {
				BOOST_FOREACH(const Command & command, transition.commands) {
					StringLiteral thisCommand(command.getFinalCommandString());
					out << innerPrefix << '['
						<< ProcessTransition::reasonMap[0][(reasonFlags & (1 << 0)) == 0 ? 0 : 1]
						<< ProcessTransition::reasonMap[1][(reasonFlags & (1 << 1)) == 0 ? 0 : 1]
						<< ProcessTransition::reasonMap[2][(reasonFlags & (1 << 2)) == 0 ? 0 : 1]
						<< "] " << thisCommand << "\n";
				}
			}
		}
		out << prefix << "}" << std::endl;
	}
	
	/**
	 * Output all command results to the given output stream.
	 *
	 * @param[in,out] out - output to this output stream
	 * @param[in,out] isFirst - set to true before the first execution for proper handling of the
	 * output formatting
	 */
	void print(std::ostream & out, bool & isFirst) {
		MissingInputSet missingInput;
		if ( isFirst ) {
			isFirst = false;
		} else {
			out << "\n\n";
		}
		out << "process : " << this->id << " {";
		bool wroteOutput = false;
		/* collect missing inputs */
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			missingInput.insert(transition.missingInput.begin(), transition.missingInput.end());
		}
		/* print missing inputs first */
		if ( ! missingInput.empty() ) {
			BOOST_FOREACH(const MissingInputSet::value_type & mi, missingInput) {
				out << "\nError: Missing input path: " << mi;
			}
			out << '\n';
			wroteOutput = true;
		}
		/* print missing outputs second */
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			if (this->transitionNeedsBuild(transition) && transition.missingInput.empty()) {
				bool allOutputsOk(true);
				BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, transition.output) {
					/* we can even check temporary files because this is performed before they are deleted */
					const std::string str(literal->getString());
					if ( ! boost::filesystem::exists(boost::filesystem::path(str, pcf::path::utf8)) ) {
						out << "\nError: Missing output path: " << str;
						allOutputsOk = false;
					}
				}
				if ( ! allOutputsOk ) {
					out << '\n';
					wroteOutput = true;
				}
			}
		}
		/* print command results finally */
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			int reasonFlags;
			if (this->transitionNeedsBuild(transition, reasonFlags) && transition.missingInput.empty()) {
				/* command was only executed if no missing input dependency was given */
				BOOST_FOREACH(const Command & command, transition.commands) {
					command.printResults(out, wroteOutput, reasonFlags);
				}
			}
		}
		if ( ! wroteOutput ) {
			out << '\n';
		}
		out << '}' << std::endl;
	}
	
	/**
	 * Returns the current process state.
	 *
	 * @return current process state
	 */
	State getState() const {
		return this->state;
	}
	
	/**
	 * Executes all transitions of this process in parallel.
	 *
	 * @param[in,out] ioService - reference to the I/O service instance for work dispatching
	 * @param[in] callProgress - callback function called to show the current progress
	 * @param[in] callNext - callback function to be executed if all tasks have been done
	 */
	bool execute(boost::asio::io_service & ioService, const ProgressCallback & callProgress, const ExecutionCallback & callNext) {
		boost::mutex::scoped_lock lock(this->mutex);
		if (this->transitionsInQueue > 0) return true; /* execution in progress */
		if (this->state != IDLE) return true; /* execution in progress */
		if ( ioService.stopped() ) {
			this->state = FAILED;
			return true;
		}
		if ( this->transitions.empty() ) {
			/* nothing to do */
			this->state = FINISHED;
			this->transitionsInQueue = 0;
			lock.unlock();
			if ( callNext ) callNext();
			return true;
		}
		this->state = RUNNING;
		this->transitionsInQueue = this->transitions.size();
		
		lock.unlock();
		BOOST_FOREACH(ProcessTransition & transition, this->transitions) {
			ioService.post(boost::phoenix::bind(
				&Process::executeTransition,
				this,
				boost::phoenix::ref(ioService),
				boost::phoenix::ref(transition),
				callProgress,
				callNext
			));
		}
		
		return true;
	}
	
	/**
	 * Updates the passed dependent map. This can be called after all transitions have been executed.
	 *
	 * @param[in,out] flatDependentMap - update this map
	 */
	void updateFlatDependentMap(PathLiteralPtrDependentMap & flatDependentMap) const {
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			if ( ! transition.output.empty() ) {
				/* remove needed dependent from flatDependentMap if its commands completed successfully */
				bool finishedOk = true;
				BOOST_FOREACH(const Command & command, transition.commands) {
					if (command.getState() != Command::FINISHED) {
						finishedOk = false;
						break;
					}
				}
				if ( finishedOk ) {
					BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, transition.output) {
						if (flatDependentMap.count(literal) > 0 && boost::filesystem::exists(boost::filesystem::path(literal->getString(), pcf::path::utf8))) {
							BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & dep, transition.dependency) {
								const std::string str(dep->getString());
								const boost::filesystem::path path(str, pcf::path::utf8);
								flatDependentMap[literal].erase(path);
							}
						}
					}
				}
			}
		}
	}
	
	/**
	 * Reset marks of the output files in the given database for temporary file deletion.
	 *
	 * @param[in,out] db - update this database
	 */
	void markOutputFilesInDatabase(Database & db) const {
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & output, transition.output) {
				db.updateFile(boost::filesystem::path(output->getString(), pcf::path::utf8), 0);
			}
		}
	}
	
	/**
	 * Marks the output files in the given database for temporary file deletion.
	 *
	 * @param[in,out] db - update this database
	 */
	void setFlagsInDatabase(Database & db) const {
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & output, transition.output) {
				const boost::filesystem::path outputPath(output->getString(), pcf::path::utf8);
				if ( ! db.setFlags(outputPath, static_cast<boost::uint64_t>(output->getFlags())) ) {
					if (this->config.verbosity >= VERBOSITY_WARN) std::cerr << "Warning: Failed to update database flags for \"" << outputPath << "\"." << std::endl;
				}
			}
		}
	}
	
	/**
	 * Cleans up incomplete transition results.
	 * 
	 * @param[in,out] out - output to this output stream
	 */
	void cleanUpIncomplete(std::ostream & out) const {
		if (this->state == FINISHED) return;
		BOOST_FOREACH(const ProcessTransition & transition, this->transitions) {
			if ( ! (transition.missingInput.empty() && this->transitionNeedsBuild(transition)) ) {
				/* there was no need to perform this transition */
				continue;
			}
			bool isComplete(true);
			BOOST_FOREACH(const Command & cmd, transition.commands) {
				if ( ! cmd.completed() ) {
					isComplete = false;
					break;
				}
			}
			if ( ! isComplete ) {
				/* remove output files for this transition */
				BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & output, transition.output) {
					const std::string outputStr(output->getString());
					const boost::filesystem::path outputPath(outputStr, pcf::path::utf8);
					if (boost::filesystem::exists(outputPath) && boost::filesystem::is_regular_file(outputPath)) {
						out << "deleting \"" << outputStr << "\": ";
						if ( boost::filesystem::remove(outputPath) ) {
							out << "ok\n";
						} else {
							out << "failed\n";
						}
					}
					output->removeFlags(PathLiteral::EXISTS);
				}
			}
		}
	}
private:
	/**
	 * Helper method to decide whether a transition needs to be build or not.
	 *
	 * @param[in] transition - check for this transition
	 * @return true if it needs to be processed, else false
	 */
	bool transitionNeedsBuild(const ProcessTransition & transition) const {
		int flags;
		return this->transitionNeedsBuild(transition, flags);
	}
	
	/**
	 * Helper method to decide whether a transition needs to be build or not.
	 *
	 * @param[in] transition - check for this transition
	 * @param[out] flags - sets the path flags for the given transition (@see ProcessTransition::Reason)
	 * @return true if it needs to be processed, else false
	 */
	bool transitionNeedsBuild(const ProcessTransition & transition, int & flags) const {
		flags = 0;
		if ( this->config.build ) {
			flags |= (1 << ProcessTransition::FORCED);
			return true;
		}
		/* always perform transition if no output is set */
		if ( transition.output.empty() ) {
			flags |= (1 << ProcessTransition::MISSING);
			return true;
		}
		/* perform transition if a dependent input file has changed or does not exist */
		BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, transition.dependency) {
			if ( literal->hasFlags(PathLiteral::MODIFIED) ) {
				flags |= (1 << ProcessTransition::CHANGED);
			}
			if ( ! (literal->hasFlags(PathLiteral::EXISTS) || literal->hasFlags(PathLiteral::TEMPORARY)) ) {
				flags |= (1 << ProcessTransition::MISSING);
			}
			if ( literal->hasFlags(PathLiteral::FORCED) ) {
				flags |= (1 << ProcessTransition::FORCED);
			}
			if (flags != 0) return true;
		}
		/* check if output exists */
		BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, transition.output) {
			if ( literal->hasFlags(PathLiteral::MODIFIED) ) {
				flags |= (1 << ProcessTransition::CHANGED);
			}
			if ( ! (literal->hasFlags(PathLiteral::EXISTS) || literal->hasFlags(PathLiteral::TEMPORARY)) ) {
				flags |= (1 << ProcessTransition::MISSING);
			}
			if ( literal->hasFlags(PathLiteral::FORCED) ) {
				flags |= (1 << ProcessTransition::FORCED);
			}
			if (flags != 0) return true;
		}
		return false;
	}

	/**
	 * Executes the given transition within the current thread by executing all its commands
	 * in sequence.
	 * 
	 * @param[in,out] ioService - reference to the I/O service instance for work dispatching
	 * @param[in,out] transition - execute this transition and update its states
	 * @param[in] callProgress - callback function called to show the current progress
	 * @param[in] callNext - callback function to be executed if all tasks have been done
	 */
	void executeTransition(boost::asio::io_service & ioService, ProcessTransition & transition, const ProgressCallback & callProgress, const ExecutionCallback & callNext) {
		/* early out if I/O service was already canceled */
		if ( ioService.stopped() ) {
			boost::mutex::scoped_lock lock(this->mutex);
			this->state = FAILED;
			return;
		}
		/* check if all needed input files are available */
		BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, transition.dependency) {
			/* temporaries can only become missing input if they are forced for creation */
			if (literal->hasFlags(PathLiteral::TEMPORARY) && ( ! literal->hasFlags(PathLiteral::FORCED) )) continue;
			const std::string str(literal->getString());
			const boost::filesystem::path path(str, pcf::path::utf8);
			if ( ! boost::filesystem::exists(path) ) {
				transition.missingInput.insert(str);
			}
		}
		if (transition.missingInput.empty() && this->transitionNeedsBuild(transition)) {
			/* execute substituted/prepared commands */
			BOOST_FOREACH(Command & command, transition.commands) {
				if (( ! command.execute(this->config.commandChecking) ) && this->config.commandChecking) {
					boost::mutex::scoped_lock lock(this->mutex);
					this->state = FAILED;
					break;
				}
			}
		}
		if ( callProgress ) callProgress(true, static_cast<boost::uint64_t>(transition.commands.size()));
		/* execution finished */
		{
			boost::mutex::scoped_lock lock(this->mutex);
			/* early out if I/O service was already canceled */
			if ( ioService.stopped() ) {
				if (this->transitionsInQueue > 0) {
					this->state = FAILED;
				} else if (this->state != FAILED) {
					this->state = FINISHED;
				}
				return;
			}
			if (this->transitionsInQueue > 0) this->transitionsInQueue--;
			if (this->transitionsInQueue <= 0) {
				if (this->state != FAILED) this->state = FINISHED;
				lock.unlock();
				if (callNext ) callNext();
			}
		}
	}
};


} /* namespace pp */


#endif /* __PP_PROCESS_HPP__ */
