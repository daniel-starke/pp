/**
 * @file Database.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-10-01
 * @version 2016-10-29
 * @remarks This implementation requires SQLite3 to be build with SQLITE_ENABLE_UPDATE_DELETE_LIMIT.
 */
#include <ctime>
#include <string>
#include <vector>
#include <boost/make_shared.hpp>
#include <boost/utility.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/time/Utility.hpp>
#include "Database.hpp"


namespace pp {


namespace {


/**
 * Local helper function to convert a given path into a generalized representation
 * for string comparison.
 *
 * @param[in] path - path to normalize
 * @return normalized path
 */
static boost::filesystem::path convertPath(const boost::filesystem::path & path) {
	return pcf::path::normalize(path, true);
}


} /* namespace */


/**
 * Internal structure with a database handle with its prepared SQL statements.
 */
struct Database::Handle {
	mutable boost::mutex mutex; /**< Database access mutex. */
	pcf::data::SQLite3 database; /**< SQLite3 database handle. */
	bool initializedDatabase; /**< True if the database was already initialized, else false. */
	pcf::data::SQLite3::Statement & updateFile; /**< Insert/update file. */
	pcf::data::SQLite3::Statement & getFile; /**< Get file information from path. */
	pcf::data::SQLite3::Statement & setFileFlags; /**< Set specific flags for single file by path. */
	pcf::data::SQLite3::Statement & setAllFileFlags; /**< Set specific flags for all files. */
	pcf::data::SQLite3::Statement & addFileFlags; /**< Add specific file flags for single file by path. */
	pcf::data::SQLite3::Statement & deleteFile; /**< Delete single file by path. */
	pcf::data::SQLite3::Statement & getFilesByFlag; /**< Returns a list of files with a specific flag set. */
	pcf::data::SQLite3::Statement & deleteFilesByFlag; /**< Delete all file with a specific flag set. */
	pcf::data::SQLite3::Statement & deleteUnusedDirectories; /**< Delete unreferenced directory paths. */
	pcf::data::SQLite3::Statement & deleteAllDirectories; /**< Delete all directory and file entries (propagated). */
	pcf::data::SQLite3::Statement & deleteAllFiles; /**< Delete all file entries. */
	pcf::data::SQLite3::Statement & getDirectoryId; /**< Get directory index by path. */
	pcf::data::SQLite3::Statement & addDirectory; /**< Add single directory by path. */
	/**
	 * Constructor.
	 * 
	 * @param[in] db - path to database
	 */
	Handle(const boost::filesystem::path & db) :
		database(db),
		/* initialize database before preparing needed SQL statements */
		initializedDatabase(this->initDatabase()),
		/* prepared statements */
		updateFile             (this->database.prepare("INSERT OR REPLACE INTO file (path, file, size, lastModified, flags) VALUES (?, ?, ?, ?, ?)")),
		getFile                (this->database.prepare("SELECT size, lastModified, flags FROM file WHERE path = ? AND file = ? LIMIT 1")),
		setFileFlags           (this->database.prepare("UPDATE OR FAIL file SET flags = ? WHERE path = ? AND file = ? LIMIT 1")),
		setAllFileFlags        (this->database.prepare("UPDATE OR FAIL file SET flags = ?")),
		addFileFlags           (this->database.prepare("UPDATE OR FAIL file SET flags = (flags | ?) WHERE path = ? AND file = ? LIMIT 1")),
		deleteFile             (this->database.prepare("DELETE FROM file WHERE path = ? AND file = ?")),
		getFilesByFlag         (this->database.prepare("SELECT (directory.path || file.file) AS path, file.size, file.lastModified, file.flags FROM file JOIN directory ON directory.id == file.path WHERE ((?001 == 0 AND flags == 0) OR (?001 != 0 AND (flags & ?001) == ?001))")),
		deleteFilesByFlag      (this->database.prepare("DELETE FROM file WHERE ((?001 == 0 AND flags == 0) OR (?001 != 0 AND (flags & ?001) == ?001))")),
		deleteUnusedDirectories(this->database.prepare("DELETE FROM directory WHERE id NOT IN (SELECT path FROM file)")),
		deleteAllDirectories   (this->database.prepare("DELETE FROM directory")),
		deleteAllFiles         (this->database.prepare("DELETE FROM file")),
		getDirectoryId         (this->database.prepare("SELECT id FROM directory WHERE path = ? LIMIT 1")),
		addDirectory           (this->database.prepare("INSERT INTO directory (path) VALUES (?)"))
	{};
private:
	/**
	 * Initialize the database structure as needed.
	 */
	bool initDatabase() {
		/* version and type of the database */
		const int databaseVersion(1);
		const std::string databaseType("parallelProcessor");
		/* helper variables */
		bool result;
		bool hasPreferenceTable = false;
		bool needsDatabaseRecreation = false;
		std::string prefKey;
		boost::mutex::scoped_lock lock(this->mutex);
		/* set database modes */
		result = this->database.query("PRAGMA foreign_keys = ON");
		if ( ! result ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::Database()
				<< pcf::exception::tag::Message("Failed to enforce foreign key constraints in database.")
			);
			return false;
		}
		result = this->database.query(std::string("PRAGMA synchronous = OFF"));
		if ( ! result ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::Database()
				<< pcf::exception::tag::Message("Failed to disable synchronous transaction mode for the database.")
			);
			return false;
		}
		result = this->database.query(std::string("PRAGMA journal_mode = MEMORY"));
		if ( ! result ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::Database()
				<< pcf::exception::tag::Message("Failed to set journal mode to memory for the database.")
			);
			return false;
		}
		/* add database tables */
		/* preference table */
		if ( this->database.query("SELECT 1 FROM preference LIMIT 1") ) {
			hasPreferenceTable = true;
		} else {
			result = this->database.query(
				"CREATE TABLE IF NOT EXISTS preference (key TEXT PRIMARY KEY UNIQUE NOT NULL, value)"
			);
			if ( ! result ) {
				BOOST_THROW_EXCEPTION(
					pcf::exception::Database()
					<< pcf::exception::tag::Message("Failed to create initial database structure for preferences.")
				);
				return false;
			}
		}
		pcf::data::SQLite3::ScopedStatement stmtGetPreferenceByKey(this->database, "SELECT key, value FROM preference WHERE key = ? LIMIT 1");
		pcf::data::SQLite3::ScopedStatement stmtSetPreferenceByKey(this->database, "INSERT OR REPLACE INTO preference (key, value) VALUES (?, ?)");
		/* check database type */
		stmtGetPreferenceByKey.reset();
		prefKey = "databaseType";
		stmtGetPreferenceByKey.bind(prefKey);
		if ( stmtGetPreferenceByKey.next() ) {
			const std::string realDatabaseType(stmtGetPreferenceByKey.getColumn<std::string>(1));
			if (realDatabaseType != databaseType) {
				BOOST_THROW_EXCEPTION(
					pcf::exception::Database()
					<< pcf::exception::tag::Message("Invalid database type \"" + realDatabaseType + "\".")
				);
				return false;
			}
		}
		/* check database version */
		stmtGetPreferenceByKey.reset();
		prefKey = "databaseVersion";
		stmtGetPreferenceByKey.bind(prefKey);
		if ( stmtGetPreferenceByKey.next() ) {
			if (stmtGetPreferenceByKey.getColumn<boost::int64_t>(1) != databaseVersion) {
				needsDatabaseRecreation = true;
			}
		} else if ( hasPreferenceTable ) {
			needsDatabaseRecreation = true;
		} else {
			stmtSetPreferenceByKey.reset();
			stmtSetPreferenceByKey.bind(prefKey);
			stmtSetPreferenceByKey.bind(static_cast<boost::int64_t>(databaseVersion));
			if ( ! stmtSetPreferenceByKey() ) {
				BOOST_THROW_EXCEPTION(
					pcf::exception::Database()
					<< pcf::exception::tag::Message("Failed to set database preference value for \"" + prefKey + "\".")
				);
				return false;
			}
		}
		if ( needsDatabaseRecreation ) {
			/* database version does not match -> recreate database */
			if ( ! this->database.clear() ) {
				BOOST_THROW_EXCEPTION(
					pcf::exception::Database()
					<< pcf::exception::tag::Message("Failed clear database.")
				);
				return false;
			}
			lock.unlock();
			return this->initDatabase();
		}
		/* directory table */
		result = this->database.query(
			"CREATE TABLE IF NOT EXISTS directory (id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT"
#ifdef PCF_IS_WIN
			" COLLATE NOCASE"
#endif /* PCF_IS_WIN */
			" UNIQUE NOT NULL)"
		);
		if ( ! result ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::Database()
				<< pcf::exception::tag::Message("Failed to create initial database structure for directories.")
			);
			return false;
		}
		/* file table */
		result = this->database.query(
			"CREATE TABLE IF NOT EXISTS file (id INTEGER PRIMARY KEY AUTOINCREMENT, path INTEGER NOT NULL, file TEXT"
#ifdef PCF_IS_WIN
			" COLLATE NOCASE"
#endif /* PCF_IS_WIN */
			" NOT NULL, size UNSIGNED BIG INT NOT NULL, lastModified DATETIME, flags UNSIGNED BIG INT NOT NULL,"
			" FOREIGN KEY(path) REFERENCES directory(id)"
			" ON DELETE CASCADE ON UPDATE CASCADE,"
			" UNIQUE(path, file) ON CONFLICT REPLACE"
			")"
		);
		if ( ! result ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::Database()
				<< pcf::exception::tag::Message("Failed to create initial database structure for files.")
			);
			return false;
		}
		return true;
	}
};



/**
 * Default constructor.
 */
Database::Database() {}


/**
 * Constructor.
 * Use database at given location.
 *
 * @param[in] db - path to database
 */
Database::Database(const boost::filesystem::path & db) {
	this->open(db);
}


/**
 * Copy constructor.
 * 
 * @param[in] o - instance to copy
 */
Database::Database(const Database & o) :
	path(o.path),
	handle(o.handle)
{}


/**
 * Destructor.
 */
Database::~Database() {
	if ( this->isOpen() ) {
		this->close();
	}
}


/**
 * Assignment operator.
 * 
 * @param[in] o - instance to assign
 * @return reference to this object for chained operations
 */
Database & Database::operator= (const Database & o) {
	if (this != (&o)) {
		this->path = o.path;
		this->handle = o.handle;
	}
	return *this;
}


/**
 * Returns the mutex of the database handle.
 *
 * @return mutex to open database
 * @throws pcf::exception::NullPointer if no database is open
 */
boost::mutex & Database::getMutex() const {
	if ( ! this->handle ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	return this->handle->mutex;
}


/**
 * Returns the path to the open database.
 *
 * @return database path
 */
boost::filesystem::path Database::getPath() const {
	return this->path;
}


/**
 * Opens the database at the given location.
 *
 * @param[in] db - path to database
 */
void Database::open(const boost::filesystem::path & db) {
	/* open and initialize database; throws an exception on error */
	this->handle = boost::make_shared<Database::Handle>(db);
	/* only set if database initialization was successful */
	this->path = db;
}


/**
 * Closes the current database instance.
 */
void Database::close() {
	this->path = boost::filesystem::path();
	this->handle.reset();
}


/**
 * Checks whether a database is currently open or not.
 * 
 * @return true if open, else false
 */
bool Database::isOpen() const {
	if ( this->path.empty() ) return false;
	if ( ! this->handle ) return false;
	if ( this->handle->database.getHandle() == NULL ) return false;
	return true;
}


/**
 * Updates the database entry for the given file by reading the meta data
 * from the file system. Sets the given file flags.
 *
 * @param[in] file - path to file
 * @param[in] flags - file flags
 * @return true if the update was successful, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::updateFile(const boost::filesystem::path & file, const boost::uint64_t flags) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	FileInformation fileInfo;
	fileInfo.path = file;
	if (boost::filesystem::exists(file) && boost::filesystem::is_regular_file(file)) {
		/* passed file does exist; get file meta data */
		fileInfo.size = static_cast<boost::uint64_t>(boost::filesystem::file_size(file));
		fileInfo.lastChange = boost::posix_time::from_time_t(boost::filesystem::last_write_time(file));
	} else {
		/* passed file does not exist */
		fileInfo.size = 0;
		fileInfo.lastChange = pcf::time::fromSqlTime(0);
	}
	fileInfo.flags = flags;
	return this->updateFile(fileInfo);
}


/**
 * Updates the database entry for the given file by using the passed meta data.
 *
 * @param[in] file - file information for database update
 * @return true if the update was successful, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::updateFile(const FileInformation & file) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	const int directoryId = this->getDirectoryIdInternal(file.path.parent_path(), true);
	if (directoryId == -1) return false;
	/* update database */
	const std::string origFileStr(convertPath(file.path).filename().generic_string(pcf::path::utf8));
	/* automatically end transaction at the end of scope */
	pcf::data::SQLite3::ScopedRollback transaction(this->handle->database);
	/* add file information to database */
	this->handle->updateFile.reset();
	this->handle->updateFile.bind(directoryId);
	this->handle->updateFile.bind(origFileStr);
	this->handle->updateFile.bind(static_cast<boost::int64_t>(file.size));
	this->handle->updateFile.bind(static_cast<boost::int64_t>(pcf::time::toSqlTime(file.lastChange)));
	this->handle->updateFile.bind(static_cast<boost::int64_t>(file.flags));
	if ( ! this->handle->updateFile() ) {
		return false;
	}
	/* save changes to the database */
	transaction.commit();
	return true;
}


/**
 * Returns the meta data for a given file.
 * 
 * @param[out] result - output variable for the meta data
 * @param[in] file - path to requested file
 * @return true if the file is in the database, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::getFile(FileInformation & result, const boost::filesystem::path & file) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	const int directoryId = this->getDirectoryIdInternal(file.parent_path(), true);
	if (directoryId == -1) return false;
	const std::string origFileStr(convertPath(file).filename().generic_string(pcf::path::utf8));
	/* get file information from database */
	this->handle->getFile.reset();
	this->handle->getFile.bind(directoryId);
	this->handle->getFile.bind(origFileStr);
	if ( this->handle->getFile.next() ) {
		/* file exists */
		result.path = file;
		result.size = static_cast<boost::uint64_t>(this->handle->getFile.getColumn<boost::int64_t>(0));
		result.lastChange = pcf::time::fromSqlTime(static_cast<boost::uint64_t>(this->handle->getFile.getColumn<boost::int64_t>(1)));
		result.flags = static_cast<boost::uint64_t>(this->handle->getFile.getColumn<boost::int64_t>(2));
		return true;
	}
	/* file not found in database */
	return false;
}


/**
 * Sets the flags for a given file.
 *
 * @param[in] file - set the flags for this file
 * @param[in] flags - flags to set
 * @return true if file exists and flags were set, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::setFlags(const boost::filesystem::path & file, const boost::uint64_t flags) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	const int directoryId = this->getDirectoryIdInternal(file.parent_path(), false);
	if (directoryId == -1) return false;
	/* update database */
	const std::string origFileStr(convertPath(file).filename().generic_string(pcf::path::utf8));
	/* automatically end transaction at the end of scope */
	pcf::data::SQLite3::ScopedRollback transaction(this->handle->database);
	/* add file information to database */
	this->handle->setFileFlags.reset();
	this->handle->setFileFlags.bind(static_cast<boost::int64_t>(flags));
	this->handle->setFileFlags.bind(directoryId);
	this->handle->setFileFlags.bind(origFileStr);
	if ( ! this->handle->setFileFlags() ) {
		return false;
	}
	/* save changes to the database */
	transaction.commit();
	return true;
}


/**
 * Sets the flags for all files.
 *
 * @param[in] flags - flags to set
 * @return true if file flags were set, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::setAllFlags(const boost::uint64_t flags) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	/* update database */
	/* automatically end transaction at the end of scope */
	pcf::data::SQLite3::ScopedRollback transaction(this->handle->database);
	/* add file information to database */
	this->handle->setAllFileFlags.reset();
	this->handle->setAllFileFlags.bind(static_cast<boost::int64_t>(flags));
	if ( ! this->handle->setAllFileFlags() ) {
		return false;
	}
	/* save changes to the database */
	transaction.commit();
	return true;
}


/**
 * Adds the passed flags to the given file.
 * 
 * @param[in] file - set the flags for this file
 * @param[in] flags - flags to add
 * @return true if file exists and flags were added, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::addFlags(const boost::filesystem::path & file, const boost::uint64_t flags) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	const int directoryId = this->getDirectoryIdInternal(file.parent_path(), false);
	if (directoryId == -1) return false;
	/* update database */
	const std::string origFileStr(convertPath(file).filename().generic_string(pcf::path::utf8));
	/* automatically end transaction at the end of scope */
	pcf::data::SQLite3::ScopedRollback transaction(this->handle->database);
	/* add file information to database */
	this->handle->addFileFlags.reset();
	this->handle->addFileFlags.bind(static_cast<boost::int64_t>(flags));
	this->handle->addFileFlags.bind(directoryId);
	this->handle->addFileFlags.bind(origFileStr);
	if ( ! this->handle->addFileFlags() ) {
		return false;
	}
	/* save changes to the database */
	transaction.commit();
	return true;
}


/**
 * Delete the given file from the database.
 * 
 * @param[in] file - file to remove from database
 * @return true if the file was removed, false on error or if no such file was found in the database
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::deleteFile(const boost::filesystem::path & file) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	const int directoryId = this->getDirectoryIdInternal(file.parent_path(), true);
	if (directoryId == -1) return false;
	const std::string origFileStr(convertPath(file).filename().generic_string(pcf::path::utf8));
	/* delete file from database */
	this->handle->deleteFile.reset();
	this->handle->deleteFile.bind(directoryId);
	this->handle->deleteFile.bind(origFileStr);
	if ( this->handle->deleteFile() ) {
		return true;
	}
	return false;
}


/**
 * Traverse all files in the database for which the given flags are set.
 *
 * @param[in] call - visitor to call
 * @param[in] flags - match files with these flags
 * @return true on success, else false if the callback returned false
 * @remarks The callback needs to return true to continue or false to abort the operation.
 * @see Callback
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::forEachFileByFlag(const Callback & call, const boost::uint64_t flags) {
	FileInformation fileInfo;
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	if ( ! call ) return true;
	boost::mutex::scoped_lock lock(this->handle->mutex);
	this->handle->getFilesByFlag.reset();
	this->handle->getFilesByFlag.bind(static_cast<boost::int64_t>(flags));
	while ( this->handle->getFilesByFlag.next() ) {
		fileInfo.path = boost::filesystem::path(this->handle->getFilesByFlag.getColumn<std::string>(0), pcf::path::utf8);
		fileInfo.size = static_cast<boost::uint64_t>(this->handle->getFilesByFlag.getColumn<boost::int64_t>(1));
		fileInfo.lastChange = pcf::time::fromSqlTime(static_cast<boost::uint64_t>(this->handle->getFilesByFlag.getColumn<boost::int64_t>(2)));
		fileInfo.flags = static_cast<boost::uint64_t>(this->handle->getFilesByFlag.getColumn<boost::int64_t>(3));
		lock.unlock(); /* return access to the database */
		if ( ! call(fileInfo) ) return false;
		lock.lock(); /* take access of the database back again */
	}
	return true;
}


/**
 * Delete all files that match the given flags.
 *
 * @param[in] flags - compare with these flags
 * @return true on success, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::deleteFilesByFlag(const boost::uint64_t flags) {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	this->handle->deleteFilesByFlag.reset();
	this->handle->deleteFilesByFlag.bind(static_cast<boost::int64_t>(flags));
	if ( this->handle->deleteFilesByFlag() ) {
		return true;
	}
	return false;
}


/**
 * Removes unnecessary directory entries from the database.
 * 
 * @return true on success, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::cleanUp() {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	if ( this->handle->deleteUnusedDirectories() ) {
		return true;
	}
	return false;
}


/**
 * Removes all entries in the database to reset it to its initial state.
 *
 * @return true on success, else false
 * @throws pcf::exception::NullPointer if no database is open
 */
bool Database::clear() {
	if ( ! this->isOpen() ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("The database was not opened.")
		);
	}
	boost::mutex::scoped_lock lock(this->handle->mutex);
	if (this->handle->deleteAllDirectories() && this->handle->deleteAllFiles()) {
		return true;
	}
	return false;
}


/**
 * Returns the index of the given directory path.
 *
 * @param[in] directory - return database index to this directory path
 * @param[in] aine - add passed directory to database if not already there (add if not exist)
 * @return directory database index or -1 on error
 * @throws pcf::exception::NullPointer if no database is open
 */
int Database::getDirectoryIdInternal(const boost::filesystem::path & directory, const bool aine) {
	std::string origDirectoryStr(convertPath(directory).generic_string(pcf::path::utf8));
	if ( origDirectoryStr.empty() ) origDirectoryStr.push_back('/');
	if (origDirectoryStr[origDirectoryStr.size() - 1] != '/') origDirectoryStr.push_back('/');
	this->handle->getDirectoryId.reset();
	this->handle->getDirectoryId.bind(origDirectoryStr);
	if ( this->handle->getDirectoryId.next() ) {
		/* directory found in database */
		return this->handle->getDirectoryId.getColumn<int>(0);
	}
	if ( ! aine ) return -1;
	this->handle->addDirectory.reset();
	this->handle->addDirectory.bind(origDirectoryStr);
	if ( ! this->handle->addDirectory() ) {
		return -1;
	}
	this->handle->getDirectoryId.reset();
	this->handle->getDirectoryId.bind(origDirectoryStr);
	if ( this->handle->getDirectoryId.next() ) {
		/* directory found in database */
		return this->handle->getDirectoryId.getColumn<int>(0);
	}
	/* no entry found and failed to add an entry */
	return -1;
}


} /* namespace pp */
