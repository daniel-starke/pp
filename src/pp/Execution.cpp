/**
 * @file Execution.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-03-22
 * @version 2016-11-18
 */
#include <fstream>
#include <sstream>
#include <boost/config/warning_disable.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/core.hpp>
#include "Execution.hpp"
#include "Utility.hpp"


namespace pp {


namespace {


/**
 * Local function to update the temporary creation flags.
 *
 * @param[in] verbosity - verbosity level
 * @param[in] temporaryFileInfoMap - update all entries in this map
 */
static void updateTemporaryCreationFlags(const TemporaryFileInfoMap & temporaryFileInfoMap, const Verbosity verbosity) {
	/* update PathLiteral::FORCED flag for each temporary within the provided map */
	BOOST_FOREACH(const TemporaryFileInfoMap::value_type & keyValue, temporaryFileInfoMap) {
		/* early out if already set for creation */
		if ( keyValue.first->hasFlags(PathLiteral::FORCED) ) {
			continue;
		}
		/* check based on whether input/output file will be created or not */
		if ( ! (keyValue.second.allInputExists && keyValue.second.allOutputExists) ) {
			/* input or output file will be created */
			keyValue.first->addFlags(PathLiteral::FORCED);
			if (verbosity >= VERBOSITY_DEBUG) {
				std::cerr << keyValue.first->getLineInfo() << ": Non-existing temporary target file will be created: " << keyValue.first->getString() << std::endl;
			}
			continue;
		} else if ( pathElementWasModified(keyValue.second.mostRecentInputChange, keyValue.second.oldestOutputChange) ) {
			/* input/output files all exist but input is younger than output */
			keyValue.first->addFlags(PathLiteral::FORCED | PathLiteral::MODIFIED);
			if (verbosity >= VERBOSITY_DEBUG) {
				std::cerr << keyValue.first->getLineInfo() << ": Temporary target file will be created due to changed input file: " << keyValue.first->getString() << std::endl;
			}
			continue;
		} else if ( keyValue.second.inputWasModified ) {
			/* permanent input dependency was/will be modified */
			keyValue.first->addFlags(PathLiteral::FORCED | PathLiteral::MODIFIED);
			if (verbosity >= VERBOSITY_DEBUG) {
				std::cerr << keyValue.first->getLineInfo() << ": Temporary target file will be created due to changed input dependency: " << keyValue.first->getString() << std::endl;
			}
			continue;
		}
	}
}


/**
 * Internal callback functor to delete remaining files from previous
 * executions which got invalid.
 */
class DeleteRemainCallback {
private:
	Database & db; /**< Referred database handle. */
	std::ostream & out; /**< Referred output stream object. */
public:
	/**
	 * Constructor.
	 * 
	 * @param[in,out] aDb - reference the used database
	 * @param[in,out] aOut - reference to the desired output stream
	 */
	explicit DeleteRemainCallback(Database & aDb, std::ostream & aOut) :
		db(aDb),
		out(aOut)
	{}
	
	/**
	 * Functor to delete the given file from the file system and write
	 * the result to the specified output stream.
	 *
	 * @param[in] fileInfo - file information object for the file which shall be deleted
	 * @return true to continue (no other return value)
	 */
	bool operator() (const FileInformation & fileInfo) {
		if ( boost::filesystem::exists(fileInfo.path) ) {
			this->out << "deleting \"" << fileInfo.path.string(pcf::path::utf8) << "\": ";
			if ( boost::filesystem::remove(fileInfo.path) ) {
				this->out << "ok\n";
			} else {
				this->out << "failed\n";
				this->db.setFlags(fileInfo.path, 0); /* try again next time */
			}
		}
		return true;
	}
};


} /* namespace */


/**
 * Executes all prepare operations on the dependency tree. This needs to be called before
 * calling Execution::execute().
 *
 * @param[in] callProgress - callback function called to show the current progress
 * @return true on success, else false
 */
bool Execution::prepare(const ProgressCallback & callProgress) {
	using namespace boost::phoenix::placeholders;
	PathLiteralPtrVector output;
	PathLiteralPtrVector duplicates;
	this->flatDependentMap.clear();
	this->temporaryFileInfoMap.clear();
	
	/* reset process nodes */
	BOOST_FOREACH(ProcessNode & node, this->processes) {
		node.traverseTopDown(boost::phoenix::bind<bool>(&Execution::resetProcessNode, _1, _2));
	}
	/* resolve dependencies */
	BOOST_FOREACH(ProcessNode & node, this->processes) {
		node.traverseDependencies(
			boost::phoenix::bind<bool>(&Execution::solveDependencies, _1, _2),
			output
		);
	}
	/* create flat dependent map */
	BOOST_FOREACH(ProcessNode & node, this->processes) {
		node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::createFlatDependentMap, this, _1, _2));
	}
	/* check for duplicates in outputs and abort with an error in such case */
	BOOST_FOREACH(ProcessNode & node, this->processes) {
		node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::checkDuplicates, _1, _2, boost::phoenix::ref(duplicates)));
		if ( ! duplicates.empty() ) {
			std::cerr << "Error: Same destination path for different inputs. Destination paths (reduced list):" << std::endl;
			const std::set<boost::shared_ptr<PathLiteral>, LessPathLiteralPtrValueLocation> dups(duplicates.begin(), duplicates.end());
			BOOST_FOREACH(const boost::shared_ptr<PathLiteral> & p, dups) {
				std::cerr << p->getLineInfo() << ": " << p->getString() << std::endl;
			}
			return false;
		}
	}
	/* create helper map to decide which temporaries need to be created and which not */
	BOOST_FOREACH(ProcessNode & node, this->processes) {
		/* remark: overlapping between different process nodes cannot happen due to the reference via pointer within the map keys */
		node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::createTemporaryInputFileInfoMap, this, _1, _2));
	}
	if ( ! this->temporaryFileInfoMap.empty() ) {
		BOOST_FOREACH(ProcessNode & node, this->processes) {
			node.traverseTopDown(boost::phoenix::bind<bool>(&Execution::createTemporaryOutputFileInfoMap, this, _1, _2));
		}
		/* update temporary creation flag based on just created helper map */
		updateTemporaryCreationFlags(this->temporaryFileInfoMap, this->config.verbosity);
		/* propagate PathLiteral::FORCED flag (set due to temporary creation check) */
		BOOST_FOREACH(ProcessNode & node, this->processes) {
			node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::propagateForcedFlag, this, _1, _2));
		}
	}
	/* prepare */
	if ( ! this->config.printOnly ) {
		/* count commands that need to be executed */
		BOOST_FOREACH(ProcessNode & node, this->processes) {
			node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::countCommands, _1, _2, callProgress));
		}
	}
	return true;
}


/**
 * Executes the command within the dependency tree in order and parallel.
 * Execution::prepare() needs to be called beforehand and Execution::complete()
 * afterwards.
 * 
 * @param[in,out] ioService - reference to the I/O service instance for work dispatching
 * @param[in] callProgress - callback function called to show the current progress
 * @param[in] callFinally - callback function to be executed if all tasks have been done
 * @return true on success, else false
 */
bool Execution::execute(boost::asio::io_service & ioService, const ProgressCallback & callProgress, const ExecutionCallback & callFinally) {
	using namespace boost::phoenix::placeholders;
	
	/* execute */
	if ( this->config.printOnly ) {
		BOOST_FOREACH(ProcessNode & node, this->processes) {
			node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::printCallback, this, _1, _2));
		}
	} else {
		boost::mutex::scoped_lock lock(this->mutex);
		this->processesInQueue = this->processes.size();
		lock.unlock();
		/* execute commands */
		BOOST_FOREACH(ProcessNode & node, this->processes) {
			node.executeChain(
				boost::phoenix::bind(&Execution::executeProcess, boost::phoenix::ref(ioService), callProgress, _1, _2),
				boost::phoenix::bind(&Execution::finished, this, callFinally)
			);
		}
	}
	return false;
}


/**
 * Executes final operations on the dependency tree like remaining file deletion and
 * log file output. This needs to be called after Execution::execute().
 *
 * @param[in,out] isFirst - needs to be set to true beforehand for internal functions
 * @return true on success, else false
 */
bool Execution::complete(bool & isFirst) {
	using namespace boost::phoenix::placeholders;
	
	if ( ! this->config.printOnly ) {
		/* print results (needs to be done first so that output file check can handle temporary files, too) */
		BOOST_FOREACH(ProcessNode & node, this->processes) {
			node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::logCallback, this, _1, _2, boost::phoenix::ref(isFirst)));
		}
		/* update flat dependent map for successfully completed commands */
		BOOST_FOREACH(ProcessNode & node, this->processes) {
			node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::updateFlatDependentMap, this, _1, _2));
		}
		/* delete temporaries if all direct dependents completed successfully */
		if ( this->config.removeTemporaries ) {
			std::ostringstream sout;
			BOOST_FOREACH(const PathLiteralPtrDependentMap::value_type & flatDependent, this->flatDependentMap) {
				if (flatDependent.first->hasFlags(PathLiteral::TEMPORARY) && flatDependent.second.empty()) {
					/* is temporary and has no remaining dependent */
					sout << "deleting \"" << flatDependent.first->getString() << "\": ";
					if ( boost::filesystem::remove(boost::filesystem::path(flatDependent.first->getString(), pcf::path::utf8)) ) {
						sout << "ok\n";
					} else {
						sout << "failed\n";
					}
				}   
			}
			const std::string output(sout.str());
			if ( ! output.empty() ) {
				std::ostream & lout(( this->log ) ? *(this->log) : std::cout);
				if ( ! isFirst ) {
					lout << "\n\n";
					isFirst = false;
				}
				lout << "remove-temporaries {\n" << output << "}" << std::endl;
			}
		}
		/* delete target files with incomplete transition */
		if ( this->config.cleanUpIncompletes ) {
			std::ostringstream sout;
			BOOST_FOREACH(ProcessNode & node, this->processes) {
				node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::cleanUpIncomplete, this, _1, _2, boost::ref(sout)));
			}
			const std::string output(sout.str());
			if ( ! output.empty() ) {
				std::ostream & lout(( this->log ) ? *(this->log) : std::cout);
				if ( ! isFirst ) {
					lout << "\n\n";
					isFirst = false;
				}
				lout << "clean-up-incompletes {\n" << output << "}" << std::endl;
			}
		}
		/* delete old remains and update database */
		if (this->config.removeRemains && this->db.isOpen()) {
			this->db.setAllFlags(1); /* reset marks */
			/* mark all output files */
			BOOST_FOREACH(ProcessNode & node, this->processes) {
				node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::markOutputFilesInDatabase, this, _1, _2));
			}
			std::ostringstream sout;
			/* file delete callback */
			this->db.forEachFileByFlag(DeleteRemainCallback(this->db, sout), 1);
			this->db.deleteFilesByFlag(1);
			/* set real flags */
			BOOST_FOREACH(ProcessNode & node, this->processes) {
				node.traverseBottomUp(boost::phoenix::bind<bool>(&Execution::setFlagsInDatabase, this, _1, _2));
			}
			const std::string output(sout.str());
			if ( ! output.empty() ) {
				std::ostream & lout(( this->log ) ? *(this->log) : std::cout);
				if ( ! isFirst ) {
					lout << "\n\n";
				}
				lout << "remove-remains {\n" << output << "}" << std::endl;
			}
			this->db.cleanUp();
		}
	}
	return true;
}


} /* namespace pp */
