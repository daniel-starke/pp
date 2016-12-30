/**
 * @file Utility.hpp
 * @author Daniel Starke
 * @copyright Copyright 2013-2016 Daniel Starke
 * @date 2013-11-23
 * @version 2016-12-23
 */
#ifndef __LIBPCFXX_PATH_UTILITY_HPP__
#define __LIBPCFXX_PATH_UTILITY_HPP__


#include <vector>
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/function.hpp>
#include <pcf/exception/General.hpp>
#include <libpcfxx.hpp>


namespace pcf {
namespace path {


/** 
 * Defines a callback interface with:
 * @param[in] string - string to check whether it contains a pattern control word
 * @return true if it contains a pattern control word, else false
 */
typedef boost::function1<bool, const std::wstring &> CheckPatternCallback;


/** 
 * Defines a callback interface with:
 * @param[in] string - unescape this pattern
 * @return unescaped pattern
 */
typedef boost::function1<std::wstring, const std::wstring &> UnescapePatternCallback;


/** 
 * Defines a callback interface with:
 * @param[in] string - match against this string
 * @param[in] pattern - match against this pattern
 * @return true on match, else false
 */
typedef boost::function2<bool, const std::wstring &, const std::wstring &> MatchingCallback;


/**
 * Code conversion facet to transform from UTF-8 to std::wstring and back.
 */
extern LIBPCFXX_DLLPORT boost::filesystem::detail::utf8_codecvt_facet utf8;


LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL correctSeparator(const boost::filesystem::path & p);
LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL normalize(const boost::filesystem::path & p, const bool correctSep = false);
LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL original(const boost::filesystem::path & p, const bool correctSep = false);
LIBPCFXX_DLLPORT bool LIBPCFXX_DECL hasBase(const boost::filesystem::path & base, const boost::filesystem::path & path);
LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL removeBase(const boost::filesystem::path & base, const boost::filesystem::path & path, const bool relative = false);
LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL resolveExecutable(const boost::filesystem::path & path);
LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL resolveExecutable(const boost::filesystem::path & path, boost::system::error_code & errorCode);
LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL getExecutablePath();
LIBPCFXX_DLLPORT boost::filesystem::path LIBPCFXX_DECL getExecutablePath(boost::system::error_code & errorCode);
LIBPCFXX_DLLPORT bool LIBPCFXX_DECL getMatchingPathList(std::vector<boost::filesystem::path> & r, const boost::filesystem::path & p, const bool matchAll, const CheckPatternCallback & hasPattern, const UnescapePatternCallback & unescapePattern, const MatchingCallback & matchesPattern);
LIBPCFXX_DLLPORT bool LIBPCFXX_DECL getWildcardPathList(std::vector<boost::filesystem::path> & r, const std::wstring & p, const bool matchAll);
LIBPCFXX_DLLPORT bool LIBPCFXX_DECL getRegexPathList(std::vector<boost::filesystem::path> & r, const std::wstring & p, const bool matchAll);


/**
 * Builds a new path from the given iterator range [start, end).
 *
 * @tparam InputIterator - obtained from boost::filesystem::path::begin()
 * @param[in,out] start - iterator start range
 * @param[in,out] end - iterator end range
 * @return new path
 */
template <typename InputIterator>
static boost::filesystem::path buildPathFromIterator(InputIterator start, InputIterator end) {
	InputIterator i = start;
	boost::filesystem::path result;
	for (; i != end; i++) {
		result /= *i;
	}
	return result;
}


/**
 * Builds a new path from the given base path and iterator range [start, end).
 *
 * @tparam InputIterator - obtained from boost::filesystem::path::begin()
 * @param[in] base - base path to use
 * @param[in,out] start - iterator start range
 * @param[in,out] end - iterator end range
 * @return new path
 */
template <typename InputIterator>
static boost::filesystem::path buildPathFromIterator(const boost::filesystem::path & base, InputIterator start, InputIterator end) {
	InputIterator i = start;
	boost::filesystem::path result(base);
	for (; i != end; i++) {
		if (*i == ".") continue;
		result /= *i;
	}
	return result;
}


/**
 * The class can be used to retrieve the access permissions
 * of the process user for a specific path.
 */
class Permission {
private:
	const boost::filesystem::path path;
	bool pExists;
	bool pRead;
	bool pWrite;
	bool pExecute;
public:
	/**
	 * Constructor.
	 * Use the getter methods to retrieve the permissions.
	 *
	 * @param[in] p - check element with this path
	 */
	explicit Permission(const boost::filesystem::path & p);
	/**
	 * Returns whether the object exists.
	 *
	 * @return object exists?
	 */
	bool exists() const {
		return this->pExists;
	}
	/**
	 * Returns whether the object can be read.
	 *
	 * @return object is readable?
	 */
	bool readable() const {
		return this->pRead;
	}
	/**
	 * Returns whether the object can be written.
	 *
	 * @return object is writable?
	 */
	bool writable() const {
		return this->pWrite;
	}
	/**
	 * Returns whether the object can be executed.
	 *
	 * @return object is executable?
	 */
	bool executable() const {
		return this->pExecute;
	}
};


} /* namespace path */
} /* namespace pcf */


#endif /* __LIBPCFXX_PATH_UTILITY_HPP__ */
