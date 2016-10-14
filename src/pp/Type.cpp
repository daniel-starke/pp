/**
 * @file Type.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-08-09
 * @version 2016-05-01
 */
#include <map>
#include <string>
#include <boost/assign/list_of.hpp>
#include <pcf/exception/General.hpp>
#include "Type.hpp"


namespace pp {


/**
 * Converts a string from the given input stream into a verbosity value.
 *
 * @param[in,out] in - read string from this input stream
 * @param[out] out - set this output variable with the converted value
 * @return reference to this object for chained operations
 * @throws pcf::exception::InvalidValue on invalid string value
 */
std::istream & operator >>(std::istream & in, Verbosity & out) {
	static const std::map<std::string, Verbosity> mapping = boost::assign::map_list_of
		("ERROR", VERBOSITY_ERROR)
		("WARN",  VERBOSITY_WARN)
		("INFO",  VERBOSITY_INFO)
		("DEBUG", VERBOSITY_DEBUG)
	;
	std::string input;
	in >> input;
	std::map<std::string, Verbosity>::const_iterator it = mapping.find(input);
	if (it != mapping.end()) {
		out = it->second;
		return in;
	}
	BOOST_THROW_EXCEPTION(
		pcf::exception::InvalidValue()
		<< pcf::exception::tag::Message(std::string("Invalid verbosity \"") + input + "\". Use ERROR, WARN, INFO or DEBUG.")
	);
	return in;
}


/**
 * Converts the given verbosity value into a string.
 *
 * @param[in,out] out - write the result to this output stream
 * @param[in] in - convert this verbosity value
 * @return reference to this object for chained operations
 */
std::ostream & operator <<(std::ostream & out, const Verbosity in) {
	static const char * mapper[] = {
		"ERROR",
		"WARN",
		"INFO",
		"DEBUG"
	};
	out << mapper[static_cast<unsigned int>(in)];
	return out;
}


/**
 * Compares two verbosity levels whereas the lowest is VERBOSITY_ERROR.
 *
 * @param[in] lhs - left hand side
 * @param[in] rhs - right hand side
 * @return true if lhs < rhs, else false
 */
bool operator <(const Verbosity lhs, const Verbosity rhs) {
	return static_cast<unsigned int>(lhs) < static_cast<unsigned int>(rhs);
}


} /* namespace pp */
