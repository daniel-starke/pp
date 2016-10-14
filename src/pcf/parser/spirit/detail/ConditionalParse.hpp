/**
 * @file ConditionalParse.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-12-14
 * @version 2016-05-01
 * @remarks Extension to the Boost Spirit2 parser framework.
 */
#ifndef __LIBPCFXX_PARSER_SPIRIT_DETAIL_CONDITIONALPARSE_HPP__
#define __LIBPCFXX_PARSER_SPIRIT_DETAIL_CONDITIONALPARSE_HPP__


#include <boost/spirit/include/qi.hpp>
#include <libpcfxx.hpp>


namespace pcf {
namespace parser {
namespace spirit {
namespace detail {


template <typename Parser, typename Iterator, typename Context, typename Skipper, typename Attribute>
inline bool conditionalParse(const Parser & aParser, Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr) {
	return aParser.parse(first, last, context, skipper, attr);
}


template <typename Parser, typename Iterator, typename Context, typename Skipper, typename Attribute>
inline bool conditionalParse(const Parser & aParser, Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr, const bool expect) {
	if ( ! expect ) {
		return aParser.parse(first, last, context, skipper, attr);
	}
	boost::spirit::qi::detail::expect_function<Iterator, Context, Skipper, boost::spirit::qi::expectation_failure<Iterator> > expectationParser(first, last, context, skipper);
	expectationParser.is_first = false;
	return ( ! expectationParser(aParser, attr) );
}


template <typename Iterator, typename Context, typename Skipper, typename Attribute>
inline bool conditionalParse(boost::spirit::unused_type /* aParser */, Iterator & /* first */, const Iterator & /* last */, Context & /* context */, const Skipper & /* skipper */, Attribute & /* attr */) {
	return true;
}


template <typename Iterator, typename Context, typename Skipper, typename Attribute>
inline bool conditionalParse(boost::spirit::unused_type /* aParser */, Iterator & /* first */, const Iterator & /* last */, Context & /* context */, const Skipper & /* skipper */, Attribute & /* attr */, const bool /* expect */) {
	return true;
}


} /* namespace detail */
} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


#endif /* __LIBPCFXX_PARSER_SPIRIT_DETAIL_CONDITIONALPARSE_HPP__ */
