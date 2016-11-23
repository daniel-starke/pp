/**
 * @file Type.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-03-22
 * @version 2016-11-20
 */
#ifndef __PP_TYPE_HPP__
#define __PP_TYPE_HPP__


#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include "Variable.hpp"
#include "Shell.hpp"


namespace pp {
	

/* forward class declarations */
class Command;
class ProcessBlock;
struct ProcessTransition;
struct TemporaryFileInfo;
class Process;
struct ProcessNode;
class Execution;


namespace parser {


template <typename Iterator, typename Skipper>
struct Script;


} /* namespace parser */


/* enumerations */
/**
 * Enumeration of possible verbosity levels.
 */
enum Verbosity {
	VERBOSITY_ERROR = 0, /**< Output only errors. */
	VERBOSITY_WARN  = 1, /**< Output errors and warnings. */
	VERBOSITY_INFO  = 2, /**< Output errors, warnings and information. */
	VERBOSITY_DEBUG = 3  /**< Output messages. */
};


/* type definitions */
typedef std::set<std::string> MissingInputSet;
typedef std::map< std::string, boost::shared_ptr<Shell> > ShellMap;
typedef std::vector<Command> CommandVector;
typedef std::vector<ProcessBlock> ProcessBlockVector;
typedef std::vector<ProcessTransition> ProcessTransitionVector;
typedef std::map<boost::shared_ptr<PathLiteral>, TemporaryFileInfo> TemporaryFileInfoMap;
typedef std::map<std::string, Process> ProcessMap;
typedef std::vector<ProcessNode> ProcessNodeVector;
typedef std::map<std::string, Execution> ExecutionMap;
typedef boost::function0<void> ExecutionCallback;
typedef boost::function2<void, const bool, const boost::uint64_t> ProgressCallback;


/* functions */
std::istream & operator>> (std::istream & in, Verbosity & out);
std::ostream & operator<< (std::ostream & out, const Verbosity in);
#ifndef _MSC_VER
/* unsupported in MSVC (@see https://connect.microsoft.com/VisualStudio/feedback/details/529700/enum-operator-overloading-broken) */
bool operator< (const Verbosity lhs, const Verbosity rhs);
#endif


/* classes */
/**
 * Structure to hold a single process transition.
 */
struct ProcessTransition {
	PathLiteralPtrVector input; /**< Input file list. */
	PathLiteralPtrVector dependency; /** Additional input dependencies. */
	PathLiteralPtrVector output; /**< Output file list. */
	MissingInputSet missingInput; /**< Missing input files after execution. */
	CommandVector commands; /**< Commands to be executed to perform this transition. */
	/** Reason flag bit positions for transitions. */
	enum Reason {
		FORCED, /**< Transition is/was forced. */
		MISSING, /**< Output or intermediate input is/was missing. */
		CHANGED /**< Input dependency was changed. */
	};
	static const char reasonMap[3][2];
};


/**
 * Structure to store details about temporary files for later removal.
 */
struct TemporaryFileInfo {
	PathLiteralPtrSet input; /**< All permanent input dependencies. */
	PathLiteralPtrSet output; /**< All permanent output dependencies. */
	boost::posix_time::ptime mostRecentInputChange; /**< Date time of the most recently changed input dependency. */
	boost::posix_time::ptime oldestOutputChange; /**< Date time of the oldest changed output file. */
	bool allInputExists; /**< All permanent output dependencies exist on the filesystem. */
	bool allOutputExists; /**< All permanent output dependencies exist on the filesystem. */
	bool inputWasModified; /**< True if the input was modified, else false. */
	
	/** Default constructor. */
	explicit TemporaryFileInfo() :
		allInputExists(true),
		allOutputExists(true),
		inputWasModified(false)
	{}
};


/**
 * This structure holds the information of a file record in the database.
 */
struct FileInformation {
	boost::filesystem::path path; /**< Path to the file. */
	boost::uint64_t size; /**< Size of the file. */
	boost::posix_time::ptime lastChange; /**< Last modification date time of the file. */
	boost::uint64_t flags; /**< User defined file flags. @see PathLiteral::Flag */
};


/**
 * Configurations for the script parser.
 */
struct Configuration {
	bool build; /**< Force build if set to true. */
	bool printOnly; /**< Do not execute, print only if set true. */
	bool commandChecking; /**< Check if the commands did exit with 0 if true. */
	bool environmentVariables; /**< Use given environment variables if true. */
	bool variableChecking; /**< Enable check whether a variable was defined before used if true. */
	bool nestedVariables; /**< Enable variable assignment within process blocks if true. */
	bool fullRecursiveMatch; /**< Enable full recursive regular expression match for input file list generation if true. */
	bool removeTemporaries; /**< Remove temporary files at the end of execution if set to true. */
	bool cleanUpIncompletes; /**< Deletes output files from incomplete transitions if true. */
	bool removeRemains; /**< Remove files added in previous runs but without any input dependency if true. */
	std::string shell; /**< Use this shell (ID of the requested shell). */
	bool lockedVerbosity; /**< Verbosity is locked for further changes by the script if set to true. */
	Verbosity verbosity; /**< Verbosity level. */
	
	/** Default constructor. */
	explicit Configuration():
		build(false),
		printOnly(false),
		commandChecking(false),
		environmentVariables(false),
		variableChecking(false),
		nestedVariables(false),
		fullRecursiveMatch(false),
		removeTemporaries(false),
		cleanUpIncompletes(false),
		removeRemains(false),
		lockedVerbosity(false),
		verbosity(VERBOSITY_INFO)
	{}
};


} /* namespace pp */


#endif /* __PP_TYPE_HPP__ */
