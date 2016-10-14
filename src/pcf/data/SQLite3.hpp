/**
 * @file SQLite3.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-01-23
 * @version 2016-05-01
 */
#ifndef __LIBPCFXX_DATA_SQLITE3_HPP__
#define __LIBPCFXX_DATA_SQLITE3_HPP__

#include <functional>
#include <string>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_set.hpp>
extern "C" {
#include <sqlite3.h>
}
#include <libpcfxx.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/data/OctetBlock.hpp>
#include <pcf/path/Utility.hpp>


namespace pcf {
namespace data {


/**
 * SQLite3 wrapper for general database access.
 */
class SQLite3 {
public:
	struct tag_null {};
	struct tag_reset {};
	/**
	 * Control tag to bind a NULL value to the SQLite3 statement.
	 */
	static const tag_null null;
	/**
	 * Control tag to reset the SQLite3 statement.
	 */
	static const tag_reset reset;
	/**
	 * Wraps the handling of an SQLite3 statement.
	 */
	class Statement : boost::noncopyable {
		friend class SQLite3;
	private:
		SQLite3 & database;
		sqlite3_stmt * statement;
		int bindIndex;
		int columnCount;
		bool needReset;
	public:
		/**
		 * Destructor.
		 */
		~Statement() {
			sqlite3_finalize(this->statement);
			this->statement = NULL;
		}
		
		/**
		 * Returns the SQLite3 internal handle.
		 *
		 * @return pointer to sqlite3_stmt object
		 */
		sqlite3_stmt * getHandle() {
			return this->statement;
		}
		
		/**
		 * Returns the SQLite3 instance reference.
		 * 
		 * @return SQLite3 instance reference
		 */
		SQLite3 & getDatabase() {
			return this->database;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - blob data
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bind(const pcf::data::OctetBlock & value, const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_blob(this->statement, idx, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - bool value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bind(const bool value, const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_int(this->statement, idx, value ? 1 : 0);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - double value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bind(const double value, const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_double(this->statement, idx, value);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - int value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bind(const int value, const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_int(this->statement, idx, value);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - int64_t value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bind(const boost::int64_t value, const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_int64(this->statement, idx, static_cast<sqlite3_int64>(value));
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - string value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bind(const std::string & value, const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_text(this->statement, idx, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - wstring value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bind(const std::wstring & value, const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_text16(this->statement, idx, value.c_str(), static_cast<int>(value.size() * 2), SQLITE_TRANSIENT);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Binds a NULL value to the specific index. The index is automatically
		 * set incremental if omitted.
		 * 
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		Statement & bindNull(const int i = 0) {
			if ( this->needReset ) this->reset();
			const int idx = (i == 0) ? this->bindIndex : i;
			const int result = sqlite3_bind_null(this->statement, idx);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot bind value: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			if (i == 0) this->bindIndex++;
			return *this;
		}
		
		/**
		 * Resets the statement to evaluate it again.
		 * This is performed automatically after each completed
		 * SQL operation.
		 * 
		 * @return reference on own object for chaining
		 */
		Statement & reset() {
			sqlite3_reset(this->statement);
			this->bindIndex = 1;
			this->columnCount = 0;
			this->needReset = false;
			return *this;
		}
		
		/**
		 * Requests the next row of the SQL result table.
		 *
		 * @return true if a row was returned, else false
		 */
		bool next() {
			if ( this->needReset ) this->reset();
			const int result = sqlite3_step(this->statement);
			if (result != SQLITE_DONE && result != SQLITE_ROW) {
				this->needReset = true;
				const std::string errMsg(std::string("Cannot process statement: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			this->columnCount = sqlite3_column_count(this->statement);
			if (result == SQLITE_DONE) {
				this->needReset = true;
				return false;
			}
			return true;
		}
		
		/**
		 * Performs the SQL statement without any result data.
		 *
		 * @return true if the SQL statement completed, else false
		 */
		bool operator ()() {
			if ( this->needReset ) this->reset();
			const int result = sqlite3_step(this->statement);
			this->needReset = true;
			if (result != SQLITE_DONE && result != SQLITE_ROW) {
				const std::string errMsg(std::string("Cannot process statement: ") + std::string(sqlite3_errmsg(this->database.getHandle())));
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
			this->columnCount = sqlite3_column_count(this->statement);
			if (result == SQLITE_DONE) {
				return true;
			}
			return false;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - blob data
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const pcf::data::OctetBlock & value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - bool value
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const bool value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - double value
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const double value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - int value
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const int value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - int64_t value
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const boost::int64_t value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - string value
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const std::string & value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - wstring value
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const std::wstring & value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the a NULL value to this SQLite3 statement.
		 * 
		 * @param[in] value - control value for NULL binding
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const SQLite3::tag_null & /* value */) {
			this->bindNull();
			return *this;
		}
		
		/**
		 * Resets the statement to evaluate it again.
		 * This is performed automatically after each completed
		 * SQL operation.
		 * 
		 * @param[in] value - control value for reset
		 * @return reference on own object for chaining
		 */
		Statement & operator <<(const SQLite3::tag_reset & /* value */) {
			this->reset();
			return *this;
		}
		
		/**
		 * Returns the number of columns in the result row.
		 *
		 * @return number of columns in result row
		 */
		int getColumnCount() const {
			return this->columnCount;
		}
		
		/**
		 * Returns the column of a specific index.
		 * The value type is set via template type.
		 *
		 * @param[in] i - index of the column
		 * @return value of the column at index i
		 * @tparam T - result value type
		 * @remarks allowed result value types are pcf::data::OctetBlock,
		 * bool, double, int, boost::int64_t and std::string, std::wstring
		 */
		template <typename T>
		T getColumn(const int i);
		
		/**
		 * Returns the value type of the column at the specific index.
		 * The result values are one defined by SQLite3
		 * (SQLITE_BLOB, SQLITE_FLOAT, SQLITE_INTEGER, SQLITE_NULL, SQLITE_TEXT).
		 *
		 * @param[in] i - index of the column
		 * @return value type of the column at index i
		 */
		int getColumnType(const int i) {
			return sqlite3_column_type(this->statement, i);
		}
		
		/**
		 * Returns the aliased name of the column at a specific index.
		 * 
		 * @param[in] i - index of the column
		 * @return name of the column at index i
		 * @tparam T - result value type
		 * @remarks allowed result value types are std::string and std::wstring
		 */
		template <typename T>
		T getColumnName(const int i);
	private:
		/**
		 * Constructor.
		 *
		 * @param[in,out] aDb - database connection instance
		 * @param[in] cmd - SQL query
		 * @remarks Statement objects are handled by the SQLite3 class and
		 * shall always be referred to by pointer or reference. A statement
		 * becomes invalid if the associated database connection is closed.
		 */
		explicit Statement(SQLite3 & aDb, const std::string & cmd) :
			database(aDb),
			statement(NULL),
			bindIndex(1),
			columnCount(0),
			needReset(false)
		{
			const int result = sqlite3_prepare_v2(this->database.getHandle(), cmd.c_str(), static_cast<int>(cmd.size() + 1), &(this->statement), NULL);
			if (result != SQLITE_OK) {
				const std::string errMsg(std::string("Cannot prepare statement: ") + cmd + "\n" + std::string(sqlite3_errmsg(this->database.getHandle())));
				sqlite3_finalize(this->statement);
				this->statement = NULL;
				BOOST_THROW_EXCEPTION(
					pcf::exception::SQLite3()
					<< pcf::exception::tag::Message(errMsg)
					<< pcf::exception::tag::ErrorCode(result)
				);
			}
		}
	};
	
	/**
	 * Scoped SQLite3 statement which is automatically removed from the data base
	 * on scope exit. The statement instances itself are still managed by SQLite3.
	 *
	 * @see pcf::data::SQLite3::Statement
	 */
	class ScopedStatement : boost::noncopyable {
	private:
		SQLite3 & database;
		Statement & statement;
	public:
		/**
		 * Constructor.
		 *
		 * @param[in,out] aDb - database connection instance
		 * @param[in] cmd - SQL query
		 * @remarks Statement objects are handled by the SQLite3 class and
		 * shall always be referred to by pointer or reference. A statement
		 * becomes invalid if the associated database connection is closed.
		 */
		explicit ScopedStatement(SQLite3 & aDb, const std::string & cmd) :
			database(aDb),
			statement(aDb.prepare(cmd))
		{}
		
		/**
		 * Destructor.
		 */
		~ScopedStatement() {
			this->database.remove(this->statement);
		}
		
		/**
		 * Returns the SQLite3 internal handle.
		 *
		 * @return pointer to sqlite3_stmt object
		 */
		sqlite3_stmt * getHandle() {
			return this->statement.getHandle();
		}
		
		/**
		 * Returns the SQLite3 instance reference.
		 * 
		 * @return SQLite3 instance reference
		 */
		SQLite3 & getDatabase() {
			return this->database;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - blob data
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bind(const pcf::data::OctetBlock & value, const int i = 0) {
			this->statement.bind(value, i);
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - bool value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bind(const bool value, const int i = 0) {
			this->statement.bind(value, i);
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - double value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bind(const double value, const int i = 0) {
			this->statement.bind(value, i);
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - int value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bind(const int value, const int i = 0) {
			this->statement.bind(value, i);
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - int64_t value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bind(const boost::int64_t value, const int i = 0) {
			this->statement.bind(value, i);
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - string value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bind(const std::string & value, const int i = 0) {
			this->statement.bind(value, i);
			return *this;
		}
		
		/**
		 * Binds the passed value to the specific index. The index is automatically
		 * set incremental if omitted.
		 *
		 * @param[in] value - wstring value
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bind(const std::wstring & value, const int i = 0) {
			this->statement.bind(value, i);
			return *this;
		}
		
		/**
		 * Binds a NULL value to the specific index. The index is automatically
		 * set incremental if omitted.
		 * 
		 * @param[in] i - index
		 * @return reference on own object for chaining
		 */
		ScopedStatement & bindNull(const int i = 0) {
			this->statement.bindNull(i);
			return *this;
		}
		
		/**
		 * Resets the statement to evaluate it again.
		 * This is performed automatically after each completed
		 * SQL operation.
		 * 
		 * @return reference on own object for chaining
		 */
		ScopedStatement & reset() {
			this->statement.reset();
			return *this;
		}
		
		/**
		 * Requests the next row of the SQL result table.
		 *
		 * @return true if a row was returned, else false
		 */
		bool next() {
			return this->statement.next();
		}
		
		/**
		 * Performs the SQL statement without any result data.
		 *
		 * @return true if the SQL statement completed, else false
		 */
		bool operator()() {
			return this->statement();
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - blob data
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const pcf::data::OctetBlock & value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - bool value
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const bool value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - double value
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const double value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - int value
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const int value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - int64_t value
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const boost::int64_t value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - string value
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const std::string & value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the passed value to this SQLite3 statement.
		 *
		 * @param[in] value - wstring value
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const std::wstring & value) {
			this->bind(value);
			return *this;
		}
		
		/**
		 * Binds the a NULL value to this SQLite3 statement.
		 * 
		 * @param[in] value - control value for NULL binding
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const SQLite3::tag_null & /* value */) {
			this->bindNull();
			return *this;
		}
		
		/**
		 * Resets the statement to evaluate it again.
		 * This is performed automatically after each completed
		 * SQL operation.
		 * 
		 * @param[in] value - control value for reset
		 * @return reference on own object for chaining
		 */
		ScopedStatement & operator <<(const SQLite3::tag_reset & /* value */) {
			this->reset();
			return *this;
		}
		
		/**
		 * Returns the number of columns in the result row.
		 *
		 * @return number of columns in result row
		 */
		int getColumnCount() const {
			return this->statement.getColumnCount();
		}
		
		/**
		 * Returns the column of a specific index.
		 * The value type is set via template type.
		 *
		 * @param[in] i - index of the column
		 * @return value of the column at index i
		 * @tparam T - result value type
		 * @remarks allowed result value types are pcf::data::OctetBlock,
		 * bool, double, int, boost::int64_t and std::string, std::wstring
		 */
		template <typename T>
		T getColumn(const int i) {
			return this->statement.getColumn<T>(i);
		}
		
		/**
		 * Returns the value type of the column at the specific index.
		 * The result values are one defined by SQLite3
		 * (SQLITE_BLOB, SQLITE_FLOAT, SQLITE_INTEGER, SQLITE_NULL, SQLITE_TEXT).
		 *
		 * @param[in] i - index of the column
		 * @return value type of the column at index i
		 */
		int getColumnType(const int i) {
			return this->statement.getColumnType(i);
		}
		
		/**
		 * Returns the aliased name of the column at a specific index.
		 * 
		 * @param[in] i - index of the column
		 * @return name of the column at index i
		 * @tparam T - result value type
		 * @remarks allowed result value types are std::string and std::wstring
		 */
		template <typename T>
		T getColumnName(const int i) {
			return this->statement.getColumnName<T>(i);
		}
	};
	
	/**
	 * Scoped SQLite3 database rollback which automatically starts a transaction and invokes a
	 * rollback on scope exit, if commitment was not requested previously.
	 */
	class ScopedRollback : boost::noncopyable {
	private:
		SQLite3 & database;
		bool doCommit;
	public:
		/**
		 * Constructor.
		 *
		 * @param[in,out] aDb - database connection instance
		 */
		explicit ScopedRollback(SQLite3 & aDb) :
			database(aDb),
			doCommit(false)
		{
			this->database.beginTransaction();
		}
		
		/**
		 * Destructor.
		 *
		 * Performs commit or rollback on the given database.
		 */
		~ScopedRollback() {
			if ( this->doCommit ) {
				/* save changes to database */
				this->database.commit();
			} else {
				/* rollback changes made at the database */
				this->database.rollback();
			}
		}
		
		/**
		 * Perform commit on scope exit.
		 */
		void commit() {
			this->doCommit = true;
		}
		
		/**
		 * Perform rollback on scope exit.
		 * This restores the default behavior after construction of this object.
		 */
		void rollback() {
			this->doCommit = false;
		}
	};
private:
	/**
	 * Comparer function object to handle a set of Statement pointers.
	 */
	struct CompareStmtPtr : public std::binary_function<Statement, Statement, bool> {
		result_type operator() (const first_argument_type & lhs, const second_argument_type & rhs) const {
			return (&lhs) < (&rhs);
		}
	};
	sqlite3 * db;
	const boost::filesystem::path file;
	boost::ptr_set<Statement, CompareStmtPtr> statements;
	Statement * beginTransactionStmt;
	Statement * rollbackStmt;
	Statement * commitStmt;
public:
	/**
	 * Constructor.
	 *
	 * Creates a new SQLite3 database connection to the database
	 * at the specified path.
	 *
	 * @param[in] path - connect to this database
	 */
	explicit SQLite3(const boost::filesystem::path & path) :
		db(NULL),
		file(path)
	{
		const int result = sqlite3_open(path.string(pcf::path::utf8).c_str(), &(this->db));
		if (result != SQLITE_OK) {
			const std::string errMsg(std::string("Cannot open database: ") + std::string(sqlite3_errmsg(this->db)));
			sqlite3_close(this->db);
			this->db = NULL;
			BOOST_THROW_EXCEPTION(
				pcf::exception::SQLite3()
				<< pcf::exception::tag::Message(errMsg)
				<< pcf::exception::tag::ErrorCode(result)
			);
		} else {
			this->beginTransactionStmt = &(this->prepare("BEGIN TRANSACTION"));
			this->rollbackStmt = &(this->prepare("ROLLBACK"));
			this->commitStmt = &(this->prepare("COMMIT"));
		}
	}
	
	/**
	 * Destructor.
	 *
	 * @remarks finalizes all SQL statements created by this->prepare().
	 */
	~SQLite3() {
		this->statements.clear();
		sqlite3_close(this->db);
		this->db = NULL;
	}
	
	/**
	 * Returns the SQLite3 internal handle.
	 *
	 * @return pointer to sqlite3 object
	 */
	sqlite3 * getHandle() {
		return this->db;
	}
	
	/**
	 * Creates a new SQL statement associated to this database connection.
	 *
	 * @param[in] cmd - SQL query
	 * @return RVALUE to the newly created SQL statement
	 * @remarks always refer to the returned statement directly via pointer or reference
	 * @see Statement
	 */
	Statement & prepare(const std::string & cmd) {
		return *(this->statements.insert(new Statement(*this, cmd)).first);
	}
	
	/**
	 * Removes and invalidates the passed SQL statement instance
	 * which was creates by SQLite3::prepare() before.
	 *
	 * @param[in,out] stmt - SQL statement handle to remove
	 */
	void remove(Statement & stmt) {
		boost::ptr_set<Statement, CompareStmtPtr>::iterator it;
		it = this->statements.find(stmt);
		if (it != this->statements.end()) this->statements.erase(it);
	}
	
	/**
	 * Use this method to query a simple SQL statement.
	 *
	 * @param[in] cmd - SQL query
	 * @return true on success, else false
	 */
	bool query(const std::string & cmd) {
		return sqlite3_exec(this->db, cmd.c_str(), NULL, NULL, NULL) == SQLITE_OK;
	}
	
	/**
	 * Clears all data within the database.
	 *
	 * @return true on success, else false
	 */
	bool clear() {
		return this->query(
			"PRAGMA writable_schema = 1;"
			"DELETE FROM sqlite_master WHERE type in ('table', 'index', 'trigger')"
			"PRAGMA writable_schema = 0;"
			"VACUUM;"
			"PRAGMA integrity_check;"
		);
	}
	
	/**
	 * Starts a transaction on the database.
	 * The transaction needs to be closed with SQLite3::commit().
	 *
	 * @return true if completed successfully, else false
	 * @see Statement::operator()()
	 */
	bool beginTransaction() {
		return (*(this->beginTransactionStmt))();
	}
	
	/**
	 * Performs a commit on the database.
	 *
	 * @return true if completed successfully, else false
	 * @see Statement::operator()()
	 */
	bool commit() {
		return (*(this->commitStmt))();
	}
	
	/**
	 * Performs a rollback on the database.
	 *
	 * @return true if completed successfully, else false
	 * @see Statement::operator()()
	 */
	bool rollback() {
		return (*(this->rollbackStmt))();
	}
};


template <> pcf::data::OctetBlock SQLite3::Statement::getColumn(const int i);
template <> bool SQLite3::Statement::getColumn(const int i);
template <> double SQLite3::Statement::getColumn(const int i);
template <> int SQLite3::Statement::getColumn(const int i);
template <> boost::int64_t SQLite3::Statement::getColumn(const int i);
template <> std::string SQLite3::Statement::getColumn(const int i);
template <> std::wstring SQLite3::Statement::getColumn(const int i);

template <> std::string SQLite3::Statement::getColumnName(const int i);
template <> std::wstring SQLite3::Statement::getColumnName(const int i);


} /* namespace data */
} /* namespace pcf */


#endif /* __LIBPCFXX_DATA_SQLITE3_HPP__ */
