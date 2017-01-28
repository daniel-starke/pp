/**
 * @file GetInfoString.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-11-27
 * @version 2016-05-01
 * @brief Function that returns a brief string representation of the given exception context.
 * @remarks Extension to the Boost Spirit2 parser framework.
 */
#ifndef __LIBPCFXX_PARSER_SPIRIT_GETINFOSTRING_HPP__
#define __LIBPCFXX_PARSER_SPIRIT_GETINFOSTRING_HPP__


#include <string>
#include <boost/spirit/include/qi.hpp>
#include <libpcfxx.hpp>


namespace pcf {
namespace parser {
namespace spirit {


namespace detail {


/** Helper callback class to retrieve the first tag of a info element. */
struct InfoRetriever {
private:
	bool first;
	std::string & out;
public:
	/**
	 * Constructor.
	 *
	 * @param[out] outString - filled with the first tag name
	 */
	InfoRetriever(std::string & outString) :
		first(true),
		out(outString)
	{}

	/**
	 * Callback method called by boost::spirit::basic_info_walker.
	 *
	 * @param[in] tag - tag of the spirit expression
	 * @param[in] value - value of the spirit expression
	 * @param[in] depth - expression depth
	 */
	void element(const boost::spirit::utf8_string & tag, const boost::spirit::utf8_string &, int /* depth */) {
		if ( ! this->first ) return;
		this->out = tag;
		this->first = false;
	}
};


} /* namespace detail */


/**
 * Returns a brief inner string representation of the expected expression.
 * 
 * @param[in] what - expected expression
 * @return brief string of the inner expected expression
 */
std::string getInnerInfoString(const boost::spirit::info & what) {
	using boost::spirit::basic_info_walker;
	std::string result;
	
	detail::InfoRetriever retriever(result);
	basic_info_walker<detail::InfoRetriever> walker(retriever, what.tag, 0);
	boost::apply_visitor(walker, what.value);
	
	if ( result.empty() ) return what.tag;
	return result;
}


/**
 * Returns a brief string representation of the expected expression.
 *
 * @param[in] what - expected expression
 * @return brief string of the expected expression
 */
std::string getInfoString(const boost::spirit::info & what) {
	using boost::spirit::basic_info_walker;
	std::string result;
	
	detail::InfoRetriever retriever(result);
	basic_info_walker<detail::InfoRetriever> walker(retriever, what.tag, 0);
	boost::apply_visitor(walker, what.value);
	
	if ( result.empty() ) return what.tag;
	return (result == what.tag) ? result : (what.tag + "(" + result + ")");
}


} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


#endif /* __LIBPCFXX_PARSER_SPIRIT_GETINFOSTRING_HPP__ */
