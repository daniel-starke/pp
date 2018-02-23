/**
 * @file Escape.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2018 Daniel Starke
 * @date 2015-08-17
 * @version 2016-05-01
 */
#ifndef __LIBPCFXX_STRING_ESCAPE_HPP__
#define __LIBPCFXX_STRING_ESCAPE_HPP__


#include <algorithm>
#include <string>
#include <boost/foreach.hpp>


namespace pcf {
namespace string {


/**
 * Escapes every occurance of a given set of characters with a specific escape character.
 * 
 * @param[in] str - string to escape
 * @param[in] escChar - escape character to use
 * @param[in] escSet - set of characters to escape
 * @return escaped string
 * @tparam CharT - character type for the string
 */
template <typename CharT>
std::basic_string<CharT> escapeCharacters(const std::basic_string<CharT> & str, const CharT escChar, const std::basic_string<CharT> & escSet) {
	std::basic_string<CharT> result;
	const typename std::basic_string<CharT>::const_iterator first(escSet.begin()), last(escSet.end());
	size_t size(0);
	BOOST_FOREACH(const CharT c, str) {
		if (std::find(first, last, c) != last) {
			size++;
		}
		size++;
	}
	result.reserve(size);
	BOOST_FOREACH(const char c, str) {
		if (std::find(first, last, c) != last) {
			result.push_back(escChar);
		}
		result.push_back(c);
	}
	return result;
}


/**
 * Escapes every occurance of a given set of characters with a specific escape character.
 * 
 * @param[in] str - string to escape
 * @param[in] escChar - escape character to use
 * @param[in] escSet - set of characters to escape
 * @return escaped string
 * @tparam CharT - character type for the string
 */
template <typename CharT>
std::basic_string<CharT> escapeCharacters(const std::basic_string<CharT> & str, const CharT escChar, const CharT * escSet) {
	std::basic_string<CharT> result;
	const CharT * first(escSet), * last(escSet + std::char_traits<CharT>::length(escSet));
	size_t size(0);
	BOOST_FOREACH(const CharT c, str) {
		if (std::find(first, last, c) != last) {
			size++;
		}
		size++;
	}
	result.reserve(size);
	BOOST_FOREACH(const char c, str) {
		if (std::find(first, last, c) != last) {
			result.push_back(escChar);
		}
		result.push_back(c);
	}
	return result;
}


} /* namespace string */
} /* namespace pcf */


#endif /* __LIBPCFXX_STRING_ESCAPE_HPP__ */
