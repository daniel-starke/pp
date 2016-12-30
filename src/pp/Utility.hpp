/**
 * @file Utility.hpp
 * @author Daniel Starke
 * @copyright Copyright 2016 Daniel Starke
 * @date 2016-11-20
 * @version 2016-12-22
 */
#ifndef __PP_UTILITY_HPP__
#define __PP_UTILITY_HPP__


#include <sstream>
#include <string>
#include <boost/config/warning_disable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <pcf/exception/General.hpp>
#include "Type.hpp"
#include "Variable.hpp"


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


/** LineInfo marker. @see pp::addLineInfoToException */
typedef boost::error_info<struct TagLineInfoMarker, bool> LineInfoMarker;


/**
 * Adds the given line information to the passed exception and marks it to prevent further additions.
 * 
 * @param[in] li - line information to add
 * @param[in] e - add to this exception and throw it again
 * @throws given exception
 * @tparam T - exception type
 */
template <typename T>
static void addLineInfoToException(const pp::LineInfo & li, const T & e) {
	if (const std::string * msg = boost::get_error_info<pcf::exception::tag::Message>(e)) {
		if (const bool * marker = boost::get_error_info<LineInfoMarker>(e)) {
			throw e; /* already has line information */
		} else {
			std::ostringstream sout;
			sout << li << ": Error: " << *msg;
			throw 
				T(e)
				<< pcf::exception::tag::Message(sout.str())
				<< LineInfoMarker(true)
			;
		}
	} else {
		throw e; /* no message contained to add line information to */
	}
}


} /* namespace pp */


#endif /* __PP_UTILITY_HPP__ */
