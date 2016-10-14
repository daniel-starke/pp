/**
 * @file Wildcard.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-11-05
 * @version 2016-05-01
 * @remarks Requires the boost_locale library.
 */
#ifndef __LIBPCFXX_STRING_WILDCARD_HPP__
#define __LIBPCFXX_STRING_WILDCARD_HPP__


#ifdef LIBPCFXX_PORTABLE
#include <algorithm>
#endif /* LIBPCFXX_PORTABLE */
#include <string>
#include <boost/locale.hpp>
#include <libpcfxx.hpp>

#ifdef LIBPCFXX_PORTABLE
#include <libpcf/typeus.h>
#endif /* LIBPCFXX_PORTABLE */
#include <libpcf/wildcardus.h>


namespace pcf {
namespace string {


namespace detail {


template <typename CharT>
inline bool matchWildcardImpl(const std::basic_string<CharT> text, const std::basic_string<CharT> pattern, const bool caseInsensitive);

template <>
inline bool matchWildcardImpl(const std::basic_string<wchar_t> text, const std::basic_string<wchar_t> pattern, const bool caseInsensitive) {
	if ( caseInsensitive ) {
#ifdef LIBPCFXX_PORTABLE
		std::wstring testLower, patternLower;
		testLower.resize(text.size());
		patternLower.resize(pattern.size());
		std::transform(text.begin(), text.end(), testLower.begin(), ::tus_toLower);
		std::transform(pattern.begin(), pattern.end(), patternLower.begin(), ::tus_toLower);
		return wcus_match(testLower.c_str(), patternLower.c_str()) == 1;
#else /* ! LIBPCFXX_PORTABLE */
		return wcus_match(boost::locale::fold_case(boost::locale::normalize(text)).c_str(), boost::locale::fold_case(boost::locale::normalize(pattern)).c_str()) == 1;
#endif /* ! LIBPCFXX_PORTABLE */
	}
#ifdef LIBPCFXX_PORTABLE
	return wcus_match(text.c_str(), pattern.c_str()) == 1;
#else /* ! LIBPCFXX_PORTABLE */
	return wcus_match(boost::locale::normalize(text).c_str(), boost::locale::normalize(pattern).c_str()) == 1;
#endif /* ! LIBPCFXX_PORTABLE */
}

template <>
inline bool matchWildcardImpl(const std::basic_string<char> text, const std::basic_string<char> pattern, const bool caseInsensitive) {
	const std::wstring wTest(boost::locale::conv::utf_to_utf<wchar_t>(text));
	const std::wstring wPattern(boost::locale::conv::utf_to_utf<wchar_t>(pattern));
	return matchWildcardImpl(wTest, wPattern, caseInsensitive);
}


} /* namespace detail */


namespace wildcardOption {
	/**
	 * Enumeration of possible wildcard matching options.
	 */
	enum Enum {
		NONE = 0x00, /**< No option (default mode). */
		CASE_INSENSITIVE = 0x01, /**< Match case insensitive (quite slow due to Unicode conversions). */
		PARTIAL_MATCH = 0x02, /**< Match a in bab for example even without explicitly writing *a*. */
		LAZY = 0x03 /**< Combined CASE_INSENSITIVE and PARTIAL_MATCH option. */
	};
	
	/**
	 * Helper operator for option combination.
	 *
	 * @param[in] lhs - first option
	 * @param[in] rhs - second option
	 * @return combination of the first and the second option
	 * @remarks Enum::NONE can not be combined with any option.
	 */
	Enum operator| (const Enum lhs, const Enum rhs) {
		return static_cast<Enum>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}
	
	/**
	 * Helper operator for option checking.
	 *
	 * @param[in] lhs - first option
	 * @param[in] rhs - second option
	 * @return intersection of the first and the second option
	 * @remarks Enum::NONE will always return Enum::NONE.
	 */
	Enum operator& (const Enum lhs, const Enum rhs) {
		return static_cast<Enum>(static_cast<int>(lhs) & static_cast<int>(rhs));
	}
	
	/**
	 * Helper operator for option checking.
	 *
	 * @param[in] lhs - first option
	 * @param[in] rhs - second option
	 * @return true if the intersection of both options is not Enum::NONE, else false
	 * @remarks Enum::NONE will always return false.
	 */
	bool operator== (const Enum lhs, const Enum rhs) {
		return static_cast<int>(lhs & rhs) != 0;
	}
} /* namespace wildcardOption */


/**
 * Matches the passed string against the given wildcard pattern.
 * Valid characters for the matching string are:
 * @li * matches any character 0 to unlimited times
 * @li ? matches any character exactly once
 * @li # matches any digit exactly once
 * Any other character will be expected to be the same as in text to match.
 *
 * @tparam CharT - character type of which the passed strings are based on
 * @param[in] str - text string
 * @param[in] cmp - wildcard matching string
 * @param[in] flag - matching options
 * @return 1 on match, else 0
 * @see wildcardOption::Enum
 */
template <typename CharT>
LIBPCFXX_DLLPORT bool LIBPCFXX_DECL matchWildcard(const std::basic_string<CharT> & str, const std::basic_string<CharT> & cmp, const wildcardOption::Enum flag = wildcardOption::NONE) {
	std::basic_string<CharT> pattern;
	if ( ! cmp.empty() ) {
		const CharT lastChar = *(cmp.rbegin());
#ifdef _MSC_VER
		if (static_cast<int>(flag & wildcardOption::PARTIAL_MATCH) != 0 && cmp[0] != '*' && cmp[0] != '?' && cmp[0] != '#' && lastChar != '*' && lastChar != '?' && lastChar != '#') {
#else
		if (flag == wildcardOption::PARTIAL_MATCH && cmp[0] != '*' && cmp[0] != '?' && cmp[0] != '#' && lastChar != '*' && lastChar != '?' && lastChar != '#') {
#endif
			pattern.push_back('*');
			pattern.append(cmp);
			pattern.push_back('*');
		} else {
			pattern = cmp;
		}
	}
#ifdef _MSC_VER
	return detail::matchWildcardImpl(str, pattern, static_cast<int>(flag & wildcardOption::CASE_INSENSITIVE) != 0);
#else
	return detail::matchWildcardImpl(str, pattern, flag == wildcardOption::CASE_INSENSITIVE);
#endif
}


} /* namespace string */
} /* namespace pcf */


#endif /* __LIBPCFXX_STRING_WILDCARD_HPP__ */
