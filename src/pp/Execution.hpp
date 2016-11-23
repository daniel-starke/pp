/**
 * @file Execution.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-03-22
 * @version 2016-11-19
 */
#ifndef __PP_EXECUTION_HPP__
#define __PP_EXECUTION_HPP__


#include <cstdlib>
#include <fstream>
#include <iosfwd>
#include <set>
#include <string>
#include <boost/asio.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/shared_ptr.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <pcf/exception/General.hpp>
#include <pcf/file/Stream.hpp>
#include <pcf/path/Utility.hpp>
#include "Database.hpp"
#include "ProcessNode.hpp"
#include "Variable.hpp"
#include "Type.hpp"


namespace pp {


/**
 * Class to maintain the dependency tree and the steps and passes
 * executed on it.
 */
class Execution {
template <typename Iterator, typename Skipper>
friend struct parser::Script;
private:
	LineInfo lineInfo; /**< Line information where this execution unit was defined. */
	ProcessNodeVector processes; /**< Dependency trees. */
	Configuration config; /**< Configuration to enforce. */
	std::string id; /**< ID of this execution unit. */
	boost::shared_ptr<pcf::file::ofstream> logFile; /**< Logging file. */
	boost::optional<std::ostream &> log; /**< Optional output stream for logging. */
	Database db; /**< Internal database handle. */
	PathLiteralPtrDependentMap flatDependentMap; /**< Flat dependency map for temporary file handling. */
	TemporaryFileInfoMap temporaryFileInfoMap; /**< Information map for temporary file handling. */
	size_t processesInQueue; /**< Number of remaining processes within the execution queue. */
	mutable boost::mutex mutex; /**< Mutex object for parallel execution. */
public:
	/** Default constructor. */
	explicit Execution():
		processes(),
		config(),
		db()
	{};
	
	/**
	 * Constructor.
	 * 
	 * @param[in] p - dependency trees to use
	 * @param[in] c - configuration to use
	 */
	explicit Execution(const ProcessNodeVector & p, const Configuration & c = Configuration()):
		processes(p),
		config(c),
		db()
	{}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	Execution(const Execution & o):
		lineInfo(o.lineInfo),
		processes(o.processes),
		config(o.config),
		id(o.id),
		logFile(o.logFile),
		log(o.log),
		db(o.db),
		flatDependentMap(o.flatDependentMap),
		temporaryFileInfoMap(o.temporaryFileInfoMap),
		processesInQueue(o.processesInQueue)
	{}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 * @return reference to this object for chained operations
	 */
	Execution & operator= (const Execution & o) {
		if (this != &o) {
			this->lineInfo = o.lineInfo;
			this->processes = o.processes;
			this->config = o.config;
			this->id = o.id;
			this->logFile = o.logFile;
			this->log = o.log;
			this->db = o.db;
			this->flatDependentMap = o.flatDependentMap;
			this->temporaryFileInfoMap = o.temporaryFileInfoMap;
			this->processesInQueue = o.processesInQueue;
		}
		return *this;
	}
	
	/**
	 * Include dependency trees of the given execution unit in this execution unit.
	 *
	 * @param[in] o - include dependency trees from this execution unit
	 */
	Execution & include(const Execution & o) {
		if (this != &o) {
			this->processes.reserve(this->processes.size() + o.processes.size());
			boost::push_back(this->processes, o.processes);
		}
		return *this;
	}
	
	/**
	 * Returns the line information where this execution unit was defined.
	 *
	 * @return defined script location
	 */
	const LineInfo & getLineInfo() const {
		return this->lineInfo;
	}
	
	/**
	 * Sets the line information where this execution unit was defined.
	 * 
	 * @param[in] li - defined script location
	 * @return reference to this object for chained operations
	 */
	Execution & setLineInfo(const LineInfo & li) {
		this->lineInfo = li;
		return *this;
	}
	
	/**
	 * Sets the default logging output.
	 *
	 * @return reference to this object for chained operations
	 */
	Execution & setLogOutput() {
		this->logFile.reset();
		this->log.reset();
		return *this;
	}
	
	/**
	 * Sets the logging output to the given output stream.
	 *
	 * @param[in,out] output - reference to a output stream for logging
	 * @return reference to this object for chained operations
	 * @remarks The given output stream object needs to be alive until this object and
	 * all its copies are destroyed.
	 */
	Execution & setLogOutput(std::ostream & output) {
		this->logFile.reset();
		this->log.reset(output);
		return *this;
	}
	
	/**
	 * Sets the logging output to the given file.
	 *
	 * @param[in] output - path to file for logging output
	 * @return reference to this object for chained operations
	 */
	Execution & setLogOutput(const boost::filesystem::path & output) {
		const std::string str(output.string(pcf::path::utf8));
		if ( str.empty() ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::InvalidValue()
				<< pcf::exception::tag::Message(std::string("Invalid log file path \"") + str + "\".")
			);
		} else if (str == "stdout") {
			this->setLogOutput(std::cout);
		} else if (str == "stderr") {
			this->setLogOutput(std::cerr);
		} else {
			try {
				if (str[0] == '+') {
					this->logFile.reset(
						new pcf::file::ofstream(
							boost::filesystem::path(++(str.begin()), str.end(), pcf::path::utf8),
							std::ofstream::app | std::ofstream::ate | std::ofstream::out
						)
					);
				} else {
					this->logFile.reset(new pcf::file::ofstream(output, std::ofstream::trunc | std::ofstream::out));
				}
				this->log.reset(*(this->logFile));
			} catch (...) {
				/* failed to open the file */
				BOOST_THROW_EXCEPTION(
					pcf::exception::Output()
					<< pcf::exception::tag::Message(std::string("Failed to create log file at \"") + output.string(pcf::path::utf8) + "\".")
				);
			}
		}
		return *this;
	}
	
	/**
	 * Opens the database at the given location for internal operations.
	 *
	 * @param[in] path - database location
	 * @return reference to this object for chained operations
	 */
	Execution & setDatabase(const boost::filesystem::path & path) {
		this->db.open(path);
		if ( ! this->db.isOpen() ) {
			/* failed to open database */
			BOOST_THROW_EXCEPTION(
				pcf::exception::InputOutput()
				<< pcf::exception::tag::Message(std::string("Failed to open database at \"") + path.string(pcf::path::utf8) + "\".")
			);
		}
		return *this;
	}
	
	bool prepare(const ProgressCallback & callProgress);
	bool execute(boost::asio::io_service & ioService, const ProgressCallback & callProgress, const ExecutionCallback & callFinally);
	bool complete(bool & isFirst);
private:
	/**
	 * Callback function to reset a given process node.
	 *
	 * @param[in,out] element - reset this node
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true on success, else false
	 */
	static bool resetProcessNode(ProcessNode::ValueType & element, const size_t /* level */) {
		element.input.clear();
		element.process.reset();
		return true;
	}
	/**
	 * Callback function to resolve the dependencies of a given process node.
	 *
	 * @param[in,out] element - resolve dependencies of this node
	 * @param[out] output - write list of output files to this variable
	 * @return true on success, else false
	 */
	static bool solveDependencies(ProcessNode::ValueType & element, PathLiteralPtrVector & output) {
		bool result;
		if ( ! element.initialInput.empty() ) {
			BOOST_FOREACH(const StringLiteral & literal, element.initialInput) {
				if ( ! element.process.createInitialInputList(literal, element.input) ) {
					return false;
				}
			}
		}
		result = element.process.createDependencyList(element.input);
		boost::push_back(output, element.process.getOutputs());
		return result;
	}
	
	/**
	 * Callback method to create the flat dependency map.
	 * 
	 * @param[in,out] element - add dependencies of this process node to the internal flat dependency map
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool createFlatDependentMap(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.createFlatDependentMap(this->flatDependentMap);
		return true;
	}
	
	/**
	 * Callback method to create the temporary input files.
	 *
	 * @param[in,out] element - add temporary input file details of this process node to the internal map
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool createTemporaryInputFileInfoMap(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.createTemporaryInputFileInfoMap(this->temporaryFileInfoMap);
		return true;
	}
	
	/**
	 * Callback method to create the temporary output files.
	 *
	 * @param[in,out] element - add temporary output file details of this process node to the internal map
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool createTemporaryOutputFileInfoMap(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.createTemporaryOutputFileInfoMap(this->temporaryFileInfoMap);
		return true;
	}
	
	/**
	 * Callback method to propagate the forced flag to all dependent process nodes.
	 *
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool propagateForcedFlag(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.propagateForcedFlag();
		return true;
	}
	
	/**
	 * Callback function to check if the given process node creates the same output files like
	 * another one.
	 *
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true if no conflict, else false
	 */
	static bool checkDuplicates(ProcessNode::ValueType & element, const size_t /* level */, PathLiteralPtrVector & duplicates) {
		std::set<boost::filesystem::path> outputs;
		/* check for all outputs that are input for this node */
		BOOST_FOREACH(boost::shared_ptr<PathLiteral> & p, element.input) {
			const boost::filesystem::path outPath(p->getString(), pcf::path::utf8);
			if ( ! outputs.insert(outPath).second ) {
				/* element was already in set */
				duplicates.push_back(p);
			}
		}
		/* check for all outputs of this node */
		outputs.clear();
		BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & p, element.process.getOutputs()) {
			const boost::filesystem::path outPath(p->getString(), pcf::path::utf8);
			if ( ! outputs.insert(outPath).second ) {
				/* element was already in set */
				duplicates.push_back(p);
			}
		}
		return duplicates.empty();
	}
	
	/**
	 * Callback method to print the commands that would be executed.
	 *
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool printCallback(ProcessNode::ValueType & element, const size_t /* level */) {
		if ( this->log ) element.process.print(*(this->log));
		else element.process.print(std::cout);
		return true;
	}
	
	/**
	 * Callback method to print the result of the executed commands.
	 *
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool logCallback(ProcessNode::ValueType & element, const size_t /* level */, bool & isFirst) {
		if ( this->log ) element.process.print(*(this->log), isFirst);
		else element.process.print(std::cout, isFirst);
		return true;
	}
	
	/**
	 * Callback function to count the number of commands to execute.
	 *
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @param[in] callProgress - callback function to handle the execution progress
	 * @return true
	 */
	static bool countCommands(ProcessNode::ValueType & element, const size_t /* level */, const ProgressCallback & callProgress) {
		if ( callProgress ) callProgress(false, element.process.countCommands());
		return true;
	}
	
	/**
	 * Callback function to execute the commands of the given process node.
	 *
	 * @param[in,out] ioService - dispatch operations to the I/O service
	 * @param[in] callProgress - callback function to handle the execution progress
	 * @param[in,out] element - element to process
	 * @param[in] callNext - callback function to be called if the execution has finished
	 * @return true
	 */
	static bool executeProcess(boost::asio::io_service & ioService, const ProgressCallback & callProgress, ProcessNode::ValueType & element, const ExecutionCallback & callNext) {
		element.process.execute(ioService, callProgress, callNext);
		return true;
	}
	
	/**
	 * Callback method to update the flat dependency map after execution has been done
	 * to decide which temporary files need to be deleted.
	 *
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool updateFlatDependentMap(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.updateFlatDependentMap(this->flatDependentMap);
		return true;
	}
	
	/**
	 * Callback method to mark all files in the database that were created to distinguish those
	 * from the remaining files of previous program executions.
	 *
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool markOutputFilesInDatabase(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.markOutputFilesInDatabase(this->db);
		return true;
	}
	
	/**
	 * Callback method to update created files in the database.
	 * 
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true
	 */
	bool setFlagsInDatabase(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.setFlagsInDatabase(this->db);
		return true;
	}
	
	/**
	 * Callback method to clean-up incomplete transitions.
	 * 
	 * @param[in,out] element - element to process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @param[in,out] out - reference to the desired output stream
	 * @return true
	 */
	bool cleanUpIncomplete(ProcessNode::ValueType & element, const size_t /* level */, std::ostream & out) {
		element.process.cleanUpIncomplete(out);
		return true;
	}
	
	/**
	 * Callback method called after each dependency tree has been completely executed.
	 *
	 * @param[in] callFinally - callback function to call after all dependency trees have been executed
	 */
	void finished(const ExecutionCallback & callFinally) {
		boost::mutex::scoped_lock lock(this->mutex);
		if (this->processesInQueue > 0) this->processesInQueue--;
		if (this->processesInQueue <= 0) {
			lock.unlock();
			if ( callFinally ) callFinally();
		}
	}
};


} /* namespace pp */


#endif /* __PP_EXECUTION_HPP__ */
