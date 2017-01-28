/**
 * @file Shell.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-09-17
 * @version 2016-05-01
 */
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/foreach.hpp>
#include <pcf/exception/General.hpp>
#include "Shell.hpp"


namespace pp {


/**
 * Adds a new replacement pattern at the end of the list of replacement pattern.
 *
 * @param[in] pattern - add this pattern
 * @return true on success, else false
 * @throws pcf::exception::SyntaxError if the syntax of the given pattern was wrong
 */
bool Shell::addReplacement(const std::string & pattern) {
	if (pattern.size() < 2) return false;
	const std::string::value_type sep(pattern[0]);
	std::vector<std::string> parts;
	boost::split(parts, pattern, boost::phoenix::placeholders::_1 == sep);
	if (parts.size() != 4) return false;
	Replacement newReplacement;
	try {
		const std::wstring regexStrW(boost::locale::conv::utf_to_utf<wchar_t>(parts[1]));
		newReplacement.search = boost::wregex(regexStrW, boost::regex_constants::perl);
	} catch (const boost::regex_error & e) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::SyntaxError()
			<< pcf::exception::tag::Message(std::string("Regular expression '") + parts[1] + "' is invalid. " + boost::locale::conv::utf_to_utf<char>(e.what()))
		);
		return false;
	}
	newReplacement.replace = boost::locale::conv::utf_to_utf<wchar_t>(parts[2]);
	this->replacement.push_back(newReplacement);
	return true;
}


/**
 * Performs all internally defined replacements on the given string and outputs the result.
 *
 * @param[in] str - apply replacement on this string
 * @return result string
 * @see http://www.boost.org/doc/libs/1_54_0/libs/regex/doc/html/boost_regex/format/boost_format_syntax.html
 */
std::string Shell::replace(const std::string & str) const {
	if ( this->replacement.empty() ) return str;
	std::wstring result(boost::locale::conv::utf_to_utf<wchar_t>(str));
	BOOST_FOREACH(const Replacement & repl, this->replacement) {
		result = boost::regex_replace(result, repl.search, repl.replace, boost::regex_constants::format_all);
	}
	return boost::locale::conv::utf_to_utf<char>(result);
}


} /* namespace pp */
