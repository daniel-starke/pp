/**
 * @file ProcessBlock.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2018 Daniel Starke
 * @date 2015-03-22
 * @version 2017-09-20
 */
#ifndef __PP_PROCESSBLOCK_HPP__
#define __PP_PROCESSBLOCK_HPP__


#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/locale.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <pcf/exception/General.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/string/Escape.hpp>
#include "Command.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "Utility.hpp"


namespace pp {


/**
 * Class to describe and handle a single process block.
 */
class ProcessBlock {
template <typename Iterator, typename Skipper>
friend struct parser::Script;
public:
	/** Possible process block types. */
	enum Type {
		UNKNOWN, /**< Unknown process block type. */
		FOREACH, /**< Process each input file in parallel. */
		ALL, /**< Combine all input files as single input. */
		NONE /**< Process regardless of the input. */
	};
private:
	/** Destination file reference list type. */
	typedef std::vector< std::vector<boost::filesystem::path> > DestinationFileReferences;
private:
	LineInfo lineInfo; /**< Line information where this process block was defined. */
	Type type; /**< Process block type. @see Type */
	VariableHandler globalVars; /**< Global variables at current scope. */
	PathVariableMap destinations; /**< Map of destination files by variable name -> path. */
	PathVariableMap dependencies; /**< Map of additional dependency files by variable name -> path. */
	CommandVector commands; /**< List of commands associated to this process block. */
	boost::wregex filter; /**< Input file filter as regular expression. */
	std::string filterStr; /**< Input file filter as string. */
	bool invertFilter; /**< Set true to invert the meaning of the filter. */
public:
	/** Default constructor. */
	explicit ProcessBlock():
		lineInfo(),
		type(UNKNOWN)
	{}
	
	/**
	 * Constructor.
	 * 
	 * @param[in] t - process block type
	 * @param[in] f - input file filter as regular expression
	 * @param[in] li - defined script location
	 */
	explicit ProcessBlock(const Type t, const std::wstring & f, const LineInfo & li = LineInfo()):
		lineInfo(li),
		type(t),
		filter(
			f,
			boost::regex_constants::perl
			| boost::regex_constants::optimize
#if defined(PCF_IS_WIN)
			| boost::regex_constants::icase
#endif /* PCF_IS_WIN */
			| boost::regex_constants::no_except
		),
		filterStr(boost::locale::conv::utf_to_utf<char>(f)),
		invertFilter(false)
	{}
	
	/**
	 * Constructor.
	 * 
	 * @param[in] t - process block type
	 * @param[in] f - input file filter as regular expression
	 * @param[in] i - set true to invert the meaning of the input file filter
	 * @param[in] li - defined script location
	 */
	explicit ProcessBlock(const Type t, const std::wstring & f, const bool i, const LineInfo & li = LineInfo()):
		lineInfo(li),
		type(t),
		filter(
			f,
			boost::regex_constants::perl
			| boost::regex_constants::optimize
#if defined(PCF_IS_WIN)
			| boost::regex_constants::icase
#endif /* PCF_IS_WIN */
			| boost::regex_constants::no_except
		),
		filterStr(boost::locale::conv::utf_to_utf<char>(f)),
		invertFilter(i)
	{}
	
	/**
	 * Copy constructor.
	 * 
	 * @param[in] o - object to copy
	 */
	ProcessBlock(const ProcessBlock & o):
		lineInfo(o.lineInfo),
		type(o.type),
		globalVars(o.globalVars),
		destinations(o.destinations),
		dependencies(o.dependencies),
		commands(o.commands),
		filter(o.filter),
		filterStr(o.filterStr),
		invertFilter(o.invertFilter)
	{}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 * @return reference to this object for chained operations
	 */
	ProcessBlock & operator= (const ProcessBlock & o) {
		if (this != &o) {
			this->lineInfo = o.lineInfo;
			this->type = o.type;
			this->globalVars = o.globalVars;
			this->destinations = o.destinations;
			this->dependencies = o.dependencies;
			this->commands = o.commands;
			this->filter = o.filter;
			this->filterStr = o.filterStr;
			this->invertFilter = o.invertFilter;
		}
		return *this;
	}
	
	/**
	 * Returns the line information where this process block was defined.
	 *
	 * @return defined script location
	 */
	const LineInfo & getLineInfo() const {
		return this->lineInfo;
	}
	
	/**
	 * Sets the line information where this process block was defined.
	 * 
	 * @param[in] li - defined script location
	 * @return reference to this object for chained operations
	 */
	ProcessBlock & setLineInfo(const LineInfo & li) {
		this->lineInfo = li;
		return *this;
	}
	
	/**
	 * Sets the input file filter as regular expression.
	 *
	 * @param[in] f - input file filter as regular expression
	 * @param[in] i - set true to invert the meaning of the input file filter
	 * @return reference to this object for chained operations
	 */
	ProcessBlock & setFilter(const std::wstring & f, const bool i = false) {
		this->filter = boost::wregex(
			f,
			boost::regex_constants::perl
			| boost::regex_constants::optimize
#if defined(PCF_IS_WIN)
			| boost::regex_constants::icase
#endif /* PCF_IS_WIN */
			| boost::regex_constants::no_except
		);
		this->filterStr = boost::locale::conv::utf_to_utf<char>(f);
		this->invertFilter = i;
		return *this;
	}
	
	/**
	 * Add a command at the end of the process block's command list.
	 *
	 * @param[in] cmd - command to add
	 */
	void addCommand(const Command & cmd) {
		this->commands.push_back(cmd);
		this->commands.back().reset();
	}
	
	/**
	 * Completes the internal destination variables by resolving remaining variable references
	 * using the passed variable handler.
	 *
	 * @param[in] vars - variable handle with variables for replacement
	 */
	void completeDestinationVariables(const VariableHandler & vars) {
		BOOST_FOREACH(PathVariableMap::value_type & destination, this->destinations) {
			std::string unknownVariable;
			destination.second.replaceVariables(unknownVariable, vars);
		}
	}
	
	/**
	 * Completes the internal commands by resolving remaining variable references
	 * using the passed variable handler.
	 *
	 * @param[in] vars - variable handle with variables for replacement
	 */
	void completeCommandVariables(const VariableHandler & vars) {
		BOOST_FOREACH(Command & command, this->commands) {
			command.replaceVariables(vars);
		}
	}
	
	/**
	 * Create a list of available variables to complete the stored commands for execution
	 * by applying the stored input file filter. This also sets the passed output variables.
	 *
	 * @param[in] in - input file list
	 * @param[out] mostRecentChange - output variable for the most recent change as date time
	 * @param[out] needBuild - output variable, set to true if the transition needs to be executed
	 * @return variable handler for variable substitution
	 */
	VariableHandler createVariables(const PathLiteralPtrVector & in, boost::posix_time::ptime & mostRecentChange, bool & needBuild) const {
		PathLiteralPtrVector filteredInput;

		BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, in) {
			const std::string str = literal->getString();
			const std::wstring wstr = boost::locale::conv::utf_to_utf<wchar_t>(str);
			const RegExNamedCaptureSet namedCaptures = getRegExCaptureNames(this->filterStr);
			boost::wsmatch what;
			
			if (boost::regex_match(wstr, what, this->filter) && ( ! this->invertFilter )) {
				VariableMap captures;
				/* overwrite previous captures with named captures from the input filter */
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
				literal->addRegexCaptures(captures);
				filteredInput.push_back(literal);
			} else if ( this->invertFilter ) {
				literal->addRegexCaptures(VariableMap());
				filteredInput.push_back(literal);
			}
		}
		
		return this->createVariablesUnfiltered(filteredInput, mostRecentChange, needBuild);
	}
	
	/**
	 * Create a list of available variables to complete the stored commands for execution.
	 * This also sets the passed output variables. An input file filter is not applied.
	 *
	 * @param[in] in - input file list
	 * @param[out] mostRecentChange - output variable for the most recent change as date time
	 * @param[out] needBuild - output variable, set to true if the transition needs to be executed
	 * @return variable handler for variable substitution
	 */
	VariableHandler createVariablesUnfiltered(const PathLiteralPtrVector & in, boost::posix_time::ptime & mostRecentChange, bool & needBuild) const {
		const char separator = ' ';
		const char quote = '"';
		VariableHandler vars;
		std::string file;
		std::string fileList, quotedFileList;
		bool gotFile;
		
		vars.addScope();
		gotFile = false;
		mostRecentChange = boost::posix_time::ptime();
		needBuild = false;
		
		BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, in) {
			const std::string str = literal->getString();
			const std::wstring wstr = boost::locale::conv::utf_to_utf<wchar_t>(str);

			const std::string quotedStr = quote + pcf::string::escapeCharacters(str, '\\', "\\\"") + quote;
			if ( ! gotFile ) {
				file = str;
				gotFile = true;
			}
			if ( ! fileList.empty() ) {
				if (*(fileList.rbegin()) != separator) {
					fileList.push_back(separator);
					fileList.append(str);
				} else {
					fileList.append(str);
				}
			} else {
				fileList = str;
			}
			if ( ! quotedFileList.empty() ) {
				if (*(quotedFileList.rbegin()) != separator) {
					quotedFileList.push_back(separator);
					quotedFileList.append(quotedStr);
				} else {
					quotedFileList.append(quotedStr);
				}
			} else {
				quotedFileList = quotedStr;
			}
			if ( ! literal->getLastModification().is_not_a_date_time() ) {
				if (mostRecentChange.is_not_a_date_time() || mostRecentChange < literal->getLastModification()) {
					mostRecentChange = literal->getLastModification();
				}
				if (literal->hasFlags(PathLiteral::MODIFIED) || ( ! (literal->hasFlags(PathLiteral::EXISTS) || literal->hasFlags(PathLiteral::TEMPORARY)) )) {
					needBuild = true;
				}
			}
		}
		
		if ( gotFile ) {
			vars.set("?", StringLiteral(file, this->lineInfo, StringLiteral::RAW));
		}
		
		vars.set("*", StringLiteral(fileList, this->lineInfo, StringLiteral::RAW));
		vars.set("@*", StringLiteral(quotedFileList, this->lineInfo, StringLiteral::RAW));
		
		return vars;
	}
	
	/**
	 * Create a list of available variables to complete the stored commands for execution
	 * by applying the stored input file filter.
	 *
	 * @param[in] in - input file list
	 * @return variable handler for variable substitution
	 */
	VariableHandler createVariables(const PathLiteralPtrVector & in) const {
		boost::posix_time::ptime mostRecentChange;
		bool needBuild;
		return this->createVariables(in, mostRecentChange, needBuild);
	}
	
	/**
	 * Creates a list of transitions from the given input file list. The build can be enforced by
	 * the passed configuration.
	 *
	 * @param[in] input - input file list
	 * @param[out] transitions - output variable to add the created transitions to
	 * @param[in] config - set Configuration::build to force build
	 * @return true on success, else false
	 */
	bool createTransitions(const PathLiteralPtrVector & input, ProcessTransitionVector & transitions, const Configuration & config) const {
		boost::posix_time::ptime mostRecentChange, mostRecentDepChange;
		bool allNeedsToBeBuild, outputDependsOnAll = false, isFirst;
		ProcessTransitionVector thisTransitions;
		PathLiteralPtrVector filteredInput;
		VariableHandler vars = this->globalVars;
		vars.clearDynamicVariables();
		
		if (this->type != NONE) {
			/* create filtered input vector */
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, input) {
				const std::string str = literal->getString();
				const std::wstring wstr = boost::locale::conv::utf_to_utf<wchar_t>(str);
				const RegExNamedCaptureSet namedCaptures = getRegExCaptureNames(this->filterStr);
				boost::wsmatch what;
				if (boost::regex_match(wstr, what, this->filter) && ( ! this->invertFilter )) {
					VariableMap captures;
					/* overwrite previous captures with named captures from the input filter */
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
					literal->addRegexCaptures(captures);
					filteredInput.push_back(literal);
				} else if ( this->invertFilter ) {
					literal->addRegexCaptures(VariableMap());
					filteredInput.push_back(literal);
				}
			}
			
			vars.addScopes(this->createVariablesUnfiltered(filteredInput, mostRecentChange, allNeedsToBeBuild).getScopes());
			
			/* check if command depends on all input files */
			outputDependsOnAll = false;
			DynamicVariableSet dynVars = boost::assign::list_of("*")("@*");
			BOOST_FOREACH(const Command & command, this->commands) {
				StringLiteral thisCommand(command.getCommandString());
				if ( thisCommand.hasDynVariable(dynVars) ) outputDependsOnAll = true;
			}
		}
		
		/* special handling for PP_THREAD */
		vars.addDynamicVariable("PP_THREAD");
		
		/* create transitions with their inputs */
		if (this->type == ALL) {
			thisTransitions.push_back(ProcessTransition());
			thisTransitions.back().input = filteredInput;
			thisTransitions.back().dependency = filteredInput;
		} else if (this->type == FOREACH) {
			BOOST_FOREACH(boost::shared_ptr<PathLiteral> & literal, filteredInput) {
				thisTransitions.push_back(ProcessTransition());
				thisTransitions.back().input.push_back(literal);
				if ( outputDependsOnAll ) {
					thisTransitions.back().dependency = filteredInput;
				} else {
					thisTransitions.back().dependency.push_back(literal);
				}
			}
		} else if (this->type == NONE) {
			thisTransitions.push_back(ProcessTransition());
			BOOST_FOREACH(ProcessTransition & transition, thisTransitions) {
				vars.addScope();
				this->setDestinationVariables(vars);
				this->addAdditionalDependencies(transition.dependency, vars, mostRecentDepChange);
				boost::optional<VariableMap &> destFiles = vars.getCurrentScope();
				if ( destFiles ) { /* always true here */
					BOOST_FOREACH(const VariableMap::value_type & variable, *destFiles) {
						/* exclude additional dependencies from destination file candidates (we still need them in vars for variable replacement) */
						if (this->dependencies.find(variable.first) != this->dependencies.end()) continue;
						const boost::filesystem::path path(variable.second.getString(), pcf::path::utf8);
						transition.output.push_back(boost::make_shared<PathLiteral>(variable.second));
						PathLiteral & output(*(transition.output.back()));
						/* if output file already exists */
						if ( boost::filesystem::exists(path) ) {
							output
								.addFlags(PathLiteral::EXISTS)
								.setLastModification(boost::posix_time::from_time_t(boost::filesystem::last_write_time(path)));
							/* check if additional input dependency was modified */
							if (( ! mostRecentDepChange.is_not_a_date_time() ) && pathElementWasModified(mostRecentDepChange, output.getLastModification())) {
								if (config.verbosity >= VERBOSITY_DEBUG) {
									std::cerr << this->lineInfo << ": Input dependency file was modified for target file: " << path.string(pcf::path::utf8) << std::endl;
								}
								output.addFlags(PathLiteral::MODIFIED);
							}
						} else {
							if (config.verbosity >= VERBOSITY_DEBUG) {
								std::cerr << this->lineInfo << ": Non-existing target file will be created: " << path.string(pcf::path::utf8) << std::endl;
							}
						}
						if (( ! output.hasFlags(PathLiteral::EXISTS) ) || config.build) {
							output.addFlags(PathLiteral::MODIFIED);
							if ( config.build ) {
								if (config.verbosity >= VERBOSITY_DEBUG) {
									std::cerr << this->lineInfo << ": Build was forced for target file: " << path.string(pcf::path::utf8) << std::endl;
								}
								output.addFlags(PathLiteral::FORCED);
							} else {
								if (config.verbosity >= VERBOSITY_DEBUG) {
									std::cerr << this->lineInfo << ": Missing target file: " << path.string(pcf::path::utf8) << std::endl;
								}
							}
						}
					}
					/* add commands to perform the transition (even if no output file is created) */
					transition.commands = this->commands;
					BOOST_FOREACH(Command & command, transition.commands) {
						command.prepare(vars, config);
					}
				}
				vars.removeScope();
			}
			/* add new transitions to output */
			boost::push_back(transitions, thisTransitions);
			return true;
		} else {
			/* error: unknown type */
			return false;
		}
		
		/* add commands and outputs to transitions */
		isFirst = true;
		BOOST_FOREACH(ProcessTransition & transition, thisTransitions) {
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & literal, transition.input) {
				vars.addScope(literal->getCaptures());
				vars.set("?", *literal);
				vars.addScope();
				this->setDestinationVariables(vars);
				this->addAdditionalDependencies(transition.dependency, vars, mostRecentDepChange);
				boost::optional<VariableMap &> destFiles = vars.getCurrentScope();
				if ( destFiles ) { /* always true here */
					bool needsToBeBuild = false;
					/* add to result list if destination files are defined and need to be build */
					/* temporaries are treated the same way like existing files */
					if (config.build || literal->hasFlags(PathLiteral::MODIFIED) || ( ! (literal->hasFlags(PathLiteral::EXISTS) || literal->hasFlags(PathLiteral::TEMPORARY)) )) {
						needsToBeBuild = true;
					} else if (this->type == ALL && (mostRecentChange.is_not_a_date_time() || allNeedsToBeBuild)) {
						needsToBeBuild = true;
					}
					if (isFirst || this->type != ALL) {
						/* only the first set of destination files are set for type ALL */
						BOOST_FOREACH(const VariableMap::value_type & variable, *destFiles) {
							/* exclude additional dependencies from destination file candidates (we still need them in vars for variable replacement) */
							if (this->dependencies.find(variable.first) != this->dependencies.end()) continue;
							const PathVariableMap::const_iterator origDestinationFile = this->destinations.find(variable.first);
							const boost::filesystem::path path(variable.second.getString(), pcf::path::utf8);
							transition.output.push_back(boost::make_shared<PathLiteral>(variable.second));
							PathLiteral & output(*(transition.output.back()));
							/* handle temporary output file */
							if (origDestinationFile != this->destinations.end()) {
								if ( origDestinationFile->second.hasFlags(PathLiteral::TEMPORARY) ) {
									output.addFlags(PathLiteral::TEMPORARY);
								}
							}
							/* handle forwarded temporary output */
							if (( ! outputDependsOnAll ) && literal->hasFlags(PathLiteral::TEMPORARY) && boost::filesystem::path(literal->getString(), pcf::path::utf8) == path) {
								output.addFlags(PathLiteral::TEMPORARY);
							}
							/* if output file already exists */
							if ( boost::filesystem::exists(path) ) {
								output
									.addFlags(PathLiteral::EXISTS)
									.setLastModification(boost::posix_time::from_time_t(boost::filesystem::last_write_time(path)));
								if (( ! literal->getLastModification().is_not_a_date_time() ) && pathElementWasModified(literal->getLastModification(), output.getLastModification())) {
									output.addFlags(PathLiteral::MODIFIED);
									if (config.verbosity >= VERBOSITY_DEBUG) {
										std::cerr << this->lineInfo << ": Input file \"" << literal->getString() << "\" was modified for target file: " << path.string(pcf::path::utf8) << std::endl;
									}
								}
								/* check if additional input dependency was modified */
								if (( ! mostRecentDepChange.is_not_a_date_time() ) && pathElementWasModified(mostRecentDepChange, output.getLastModification())) {
									output.addFlags(PathLiteral::MODIFIED);
									if (config.verbosity >= VERBOSITY_DEBUG) {
										std::cerr << this->lineInfo << ": Input dependency file was modified for target file: " << path.string(pcf::path::utf8) << std::endl;
									}
								}
							} else if ( output.hasFlags(PathLiteral::TEMPORARY) ) {
								/* non-existing temporary output keeps track of most recent input/dependency modification to aid temporary creation decision */
								if (outputDependsOnAll || this->type == ALL) {
									if (( ! (mostRecentChange.is_not_a_date_time() || mostRecentDepChange.is_not_a_date_time()) ) && mostRecentDepChange > mostRecentChange) {
										output.setLastModification(mostRecentDepChange);
									} else {
										output.setLastModification(mostRecentChange);
									}
								} else {
									if (( ! (mostRecentChange.is_not_a_date_time() || mostRecentDepChange.is_not_a_date_time()) ) && mostRecentDepChange > mostRecentChange) {
										output.setLastModification(mostRecentDepChange);
									} else {
										output.setLastModification(literal->getLastModification());
									}
								}
							} else {
								if (config.verbosity >= VERBOSITY_DEBUG) {
									std::cerr << this->lineInfo << ": Non-existing target file will be created: " << path.string(pcf::path::utf8) << std::endl;
								}
							}
							/* output needs to be build */
							if (config.build || literal->hasFlags(PathLiteral::FORCED)) {
								output.addFlags(PathLiteral::FORCED | PathLiteral::MODIFIED);
								if (config.verbosity >= VERBOSITY_DEBUG) {
									std::cerr << this->lineInfo << ": Missing target file: " << path.string(pcf::path::utf8) << std::endl;
								}
							}
							if ( needsToBeBuild ) {
								output.addFlags(PathLiteral::MODIFIED);
								if (config.verbosity >= VERBOSITY_DEBUG) {
									std::cerr << this->lineInfo << ": Input file was modified for target file: " << path.string(pcf::path::utf8) << std::endl;
								}
							}
						}
						/* add commands to perform the transition (even if no output file is created) */
						transition.commands = this->commands;
						BOOST_FOREACH(Command & command, transition.commands) {
							command.prepare(vars, config);
						}
					}
				}
				vars.removeScope();
				vars.removeScope();
				isFirst = false;
			}
		}
		
		/* add new transitions to output */
		boost::push_back(transitions, thisTransitions);
		
		return true;
	}
private:
	/**
	 * Completes the internal destination file string by replacing known variables. Checking can be
	 * enabled to fail on missing substitutions.
	 *
	 * @param[in,out] vars - add destination files to the variable handle and use included variables
	 * to substitute referenced variables
	 * @param[in] variableChecking - enable strong variable checking
	 */
	void setDestinationVariables(VariableHandler & vars, const bool variableChecking = true) const {
		BOOST_FOREACH(const PathVariableMap::value_type & variable, this->destinations) {
			/* included variable references are automatically replaces */
			VariableHandler::Checking checking = VariableHandler::CHECKING_WARN;
			if ( variableChecking ) {
				checking = VariableHandler::CHECKING_ERROR;
			}
			vars.set(variable.first, variable.second, checking);
		}
	}
	
	/**
	 * Completes the internal dependency file string by replacing known variables. Checking can be
	 * enabled to fail on missing substitutions.
	 *
	 * @param[in,out] target - add dependency files to this vector
	 * @param[in,out] vars - add dependency files to the variable handle and use included variables
	 * to substitute referenced variables
	 * @param[out] mostRecentChange - output variable for the most recent change as date time
	 * @param[in] variableChecking - enable strong variable checking
	 */
	void addAdditionalDependencies(PathLiteralPtrVector & target, VariableHandler & vars, boost::posix_time::ptime & mostRecentChange, const bool variableChecking = true) const {
		BOOST_FOREACH(const PathVariableMap::value_type & variable, this->dependencies) {
			/* included variable references are automatically replaces */
			VariableHandler::Checking checking = VariableHandler::CHECKING_WARN;
			if ( variableChecking ) {
				checking = VariableHandler::CHECKING_ERROR;
			}
			target.push_back(boost::make_shared<PathLiteral>(vars.set(variable.first, variable.second, checking)));
			const std::string depFileStr(target.back()->getString());
			const boost::filesystem::path path(depFileStr, pcf::path::utf8);
			if ( boost::filesystem::exists(path) ) {
				PathLiteral & output(*(target.back()));
				output
					.addFlags(PathLiteral::EXISTS)
					.setLastModification(boost::posix_time::from_time_t(boost::filesystem::last_write_time(path)));
				if (mostRecentChange.is_not_a_date_time() || mostRecentChange < output.getLastModification()) {
					mostRecentChange = output.getLastModification();
				}
			} else {
				BOOST_THROW_EXCEPTION(
					pcf::exception::FileNotFound()
					<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(target.back()->getLineInfo()) + ": Dependency file not found \"" + depFileStr + "\".")
				);
			}
		}
	}
};


} /* namespace pp */


#endif /* __PP_PROCESSBLOCK_HPP__ */
