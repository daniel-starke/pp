/**
 * @file SQLite3.cpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2018 Daniel Starke
 * @date 2014-01-23
 * @version 2017-01-14
 */
#include <string>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
extern "C" {
#include <sqlite3.h>
}
#include <libpcfxx.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/data/OctetBlock.hpp>
#include <pcf/data/SQLite3.hpp>
#include <pcf/path/Utility.hpp>


namespace pcf {
namespace data {


template <>
pcf::data::OctetBlock SQLite3::Statement::getColumn(const int i) {
	const boost::uint8_t * byteData = reinterpret_cast<const boost::uint8_t *>(sqlite3_column_blob(this->statement, i));
	const int byteCount = static_cast<int>(sqlite3_column_bytes(this->statement, i));
	if (byteCount < 1) return pcf::data::OctetBlock();
	return pcf::data::OctetBlock(byteData, byteData + byteCount);
}


template <>
bool SQLite3::Statement::getColumn(const int i) {
	return (sqlite3_column_int(this->statement, i) != 0);
}


template <>
double SQLite3::Statement::getColumn(const int i) {
	return sqlite3_column_double(this->statement, i);
}


template <>
int SQLite3::Statement::getColumn(const int i) {
	return sqlite3_column_int(this->statement, i);
}


template <>
boost::int64_t SQLite3::Statement::getColumn(const int i) {
	return static_cast<boost::int64_t>(sqlite3_column_int64(this->statement, i));
}


template <>
std::string SQLite3::Statement::getColumn(const int i) {
	const char * byteData = reinterpret_cast<const char *>(sqlite3_column_text(this->statement, i));
	const int byteCount = static_cast<int>(sqlite3_column_bytes(this->statement, i));
	if (byteCount < 1) return std::string();
	return std::string(byteData, byteData + byteCount);
}


template <>
std::wstring SQLite3::Statement::getColumn(const int i) {
	const char * byteData = reinterpret_cast<const char *>(sqlite3_column_text16(this->statement, i));
	const int byteCount = static_cast<int>(sqlite3_column_bytes16(this->statement, i));
	if (byteCount < 1) return std::wstring();
	return std::wstring(reinterpret_cast<const wchar_t *>(byteData), reinterpret_cast<const wchar_t *>(byteData + byteCount));
}


template <>
std::string SQLite3::Statement::getColumnName(const int i) {
	const char * columnName = sqlite3_column_name(this->statement, i);
	if (columnName == NULL) return std::string();
	return std::string(columnName);
}


template <>
std::wstring SQLite3::Statement::getColumnName(const int i) {
	const void * columnName = sqlite3_column_name16(this->statement, i);
	if (columnName == NULL) return std::wstring();
	return std::wstring(reinterpret_cast<const wchar_t *>(columnName));
}


} /* namespace data */
} /* namespace pcf */
