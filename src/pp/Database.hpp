/**
 * @file Database.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-10-01
 * @version 2016-05-01
 */
#ifndef __PP_DATABASE_HPP__
#define __PP_DATABASE_HPP__


#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <pcf/data/SQLite3.hpp>
#include "Type.hpp"


namespace pp {


/**
 * Class to handle database access.
 */
class Database {
public:
	/** Callback function type for forEachFileByFlag(). */
	typedef boost::function1<bool, const FileInformation &> Callback;
private:
	struct Handle; /**< Forward declaration of internal database structure. */
	boost::filesystem::path path; /**< Path to used database. */
	boost::shared_ptr<Handle> handle; /**< Internal database handle and statements. */
public:
	explicit Database();
	explicit Database(const boost::filesystem::path & db);
	Database(const Database & o);
	~Database();
	Database & operator= (const Database & o);
	boost::mutex & getMutex() const;
	boost::filesystem::path getPath() const;
	void open(const boost::filesystem::path & db);
	void close();
	bool isOpen() const;
	bool updateFile(const boost::filesystem::path & file, const boost::uint64_t flags = 0);
	bool updateFile(const FileInformation & file);
	bool getFile(FileInformation & result, const boost::filesystem::path & file);
	bool setFlags(const boost::filesystem::path & file, const boost::uint64_t flags);
	bool setAllFlags(const boost::uint64_t flags);
	bool addFlags(const boost::filesystem::path & file, const boost::uint64_t flags);
	bool deleteFile(const boost::filesystem::path & file);
	bool forEachFileByFlag(const Callback & call, const boost::uint64_t flags);
	bool deleteFilesByFlag(const boost::uint64_t flags);
	bool cleanUp();
	bool clear();
private:
	int getDirectoryIdInternal(const boost::filesystem::path & directory, const bool aine = false);
};


} /* namespace pp */


#endif /* __PP_DATABASE_HPP__ */
