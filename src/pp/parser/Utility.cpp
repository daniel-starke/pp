/**
 * @file Utility.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-12-10
 * @version 2016-05-01
 */
#include "Utility.hpp"


namespace pp {
namespace parser {


/**
 * Throws a pcf::exception::SyntaxError by formatting the given error messages and setting the
 * script file location from the given string literal.
 *
 * @param[in] regexStr - extract script location and regular expression string from this iterator
 * @param[in] e - regular expression syntax error
 */
void throwRegexException(const pp::StringLiteral & regexStr, const boost::regex_error & e) {
	std::ostringstream sout;
	if ( ! regexStr.getLineInfo().file.empty() ) {
		sout << regexStr.getLineInfo() << ": ";
	}
	sout << "Error: Regular expression '" << regexStr.getString() << "' is invalid.\n" << e.what();
	BOOST_THROW_EXCEPTION(
		pcf::exception::SyntaxError()
		<< pcf::exception::tag::Message(sout.str())
	);
}


/**
 * Parses and returns the index of a given array variable.
 *
 * @param[in] input - parse this variable name
 * @param[in] variable - name of the variable
 * @return optional index of the variable, if there was any and -1 if no index was set
 */
boost::optional<ssize_t> parseVariableIndex(const std::string & input, const std::string & variable) {
	namespace qi = boost::spirit::qi;
	std::string::const_iterator start(input.begin());
	const std::string::const_iterator end(input.end());
	boost::optional<size_t> result;
	try {
		if ( qi::parse(start, end, qi::lit(variable) >> -("[" >> qi::uint_ >> "]"), result) ) {
			if (start == end) {
				if ( result ) {
					return boost::optional<ssize_t>(static_cast<ssize_t>(*result));
				} else {
					return boost::optional<ssize_t>(-1);
				}
			}return boost::optional<ssize_t>();
		}
	} catch (...) {}
	return boost::optional<ssize_t>();
}


} /* namespace parser */
} /* namespace pp */
