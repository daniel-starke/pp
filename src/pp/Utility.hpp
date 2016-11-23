/**
 * @file Utility.hpp
 * @author Daniel Starke
 * @copyright Copyright 2016 Daniel Starke
 * @date 2016-11-20
 * @version 2016-11-20
 */
#ifndef __PP_UTILITY_HPP__
#define __PP_UTILITY_HPP__


#include <string>
#include <boost/config/warning_disable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include "Type.hpp"


namespace pp {


/**
 * Checks if the given file time indicates that the file was changed compared to the specified
 * reference time. An tolerance of 1 second if used. The function always returns false if no
 * valid time or reference time was given.
 * 
 * @param[in] time - file modification time
 * @param[in] ref - reference file modification time (e.g. input file)
 * @return true if modified, else false and also on error
 */
static bool pathElementWasModified(const boost::posix_time::ptime & time, const boost::posix_time::ptime & ref) {
	if ( time.is_special() ) return false;
	if ( ref.is_special() ) return false;
	return (time >= (ref + boost::posix_time::seconds(1)));
}


/**
 * Reduces groups of consecutive slashes to a single slash. This can be used to normalize Unix paths.
 * 
 * @param[in] str - reduces slashes in this string
 * @return string with reduced consecutive slashes
 */
template <typename CharT>
static std::basic_string<CharT> reduceConsecutiveSlashes(const std::basic_string<CharT> & str) {
	CharT last(0);
	std::basic_string<CharT> result;
	result.reserve(str.size());
	BOOST_FOREACH(const CharT c, str) {
		if (c == '/') {
			if (last != c) {
				result.push_back(c);
				last = c;
			}
		} else {
			result.push_back(c);
			last = c;
		}
	}
	return result;
}


} /* namespace pp */


#endif /* __PP_UTILITY_HPP__ */
