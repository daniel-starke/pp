/**
 * @file Convert.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-09-17
 * @version 2016-05-01
 */
#ifndef __LIBPCFXX_STRING_CONVERT_HPP__
#define __LIBPCFXX_STRING_CONVERT_HPP__


#include <string>
#include <boost/algorithm/string.hpp>
#include <pcf/exception/General.hpp>


namespace pcf {
namespace string {


/**
 * Converts the given string to a boolean value or throws an exception on error.
 * 
 * @param[in] str - string to parse
 * @return parsed boolean value
 * @throw pcf::exception::InvalidValue on invalid input value
 * @tparam CharT - character type for the string
 */
template <typename CharT>
bool toBool(const std::basic_string<CharT> & str) {
	const std::basic_string<CharT> tmp(boost::algorithm::to_lower_copy(str));
	if (tmp == "true") {
		return true;
	} else if (tmp == "false") {
		return false;
	} else {
		BOOST_THROW_EXCEPTION(
			pcf::exception::InvalidValue()
			<< pcf::exception::tag::Message(std::string("Given string \"") + str + "\" is not a boolean value.")
		);
	}
	return false;
}


} /* namespace string */
} /* namespace pcf */


#endif /* __LIBPCFXX_STRING_ESCAPE_HPP__ */
