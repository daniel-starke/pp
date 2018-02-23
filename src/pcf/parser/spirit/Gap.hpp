/**
 * @file Gap.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2018 Daniel Starke
 * @date 2015-11-27
 * @version 2016-11-20
 * @brief Enables single skip parser invocation as gap and gap(p) for explicit skipping.
 * @remarks Extension to the Boost Spirit2 parser framework.
 * @remarks Only works within a disabled skipper context (e.g. within lexeme[]) or within phrase_parse if
 * qi::skip_flag::dont_postskip is set. Some parser however, like char_, so not revert the iterator to the
 * position before the skip parser when parsing fails. An and-prediction can be used in these cases.
 */
#ifndef __LIBPCFXX_PARSER_SPIRIT_GAP_HPP__
#define __LIBPCFXX_PARSER_SPIRIT_GAP_HPP__


#include <string>
#include <boost/spirit/include/qi.hpp>
#include <libpcfxx.hpp>


namespace pcf {
namespace parser {
namespace spirit {


/* define the placeholder representing our new component when building parser expressions */
BOOST_SPIRIT_TERMINAL_EX(gap)


} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


namespace boost {
namespace spirit {


/** enables gap as terminal */
template <>
struct use_terminal<qi::domain, pcf::parser::spirit::tag::gap>
	: mpl::true_
{};


/** enables gap(p) as terminal */
template <typename A0>
struct use_terminal<qi::domain, terminal_ex<pcf::parser::spirit::tag::gap, fusion::vector1<A0> > >
	: boost::spirit::traits::matches<qi::domain, A0>
{};


/** enables *lazy* gap(p) as terminal */
template <>
struct use_lazy_terminal<qi::domain, pcf::parser::spirit::tag::gap, 1 /* arity */>
	: mpl::true_
{};


} /* namespace spirit */
} /* namespace boost */


namespace pcf {
namespace parser {
namespace spirit {
namespace detail {


#ifndef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
using pcf::parser::spirit::gap;
#endif
using pcf::parser::spirit::gap_type;


template <typename Iterator, typename T>
inline bool skipGap(Iterator & first, const Iterator & last, const T & skipper) {
	return first != last && skipper.parse(first, last, boost::spirit::qi::unused, boost::spirit::qi::unused, boost::spirit::qi::unused);
}

template <typename Iterator>
inline bool skipGap(Iterator &, const Iterator &, boost::spirit::qi::unused_type) {
	return true;
}

template <typename Iterator, typename Skipper>
inline bool skipGap(Iterator & first, const Iterator & last, const boost::spirit::qi::detail::unused_skipper<Skipper> & skipper) {
	return skipGap(first, last, boost::spirit::qi::detail::get_skipper(skipper));
}


/** implementation of the parser component */
struct GapPrimitive : boost::spirit::qi::primitive_parser<GapPrimitive> {
	/** define the attribute type exposed by this parser component */
	template <typename Context, typename Iterator>
	struct attribute {
		typedef boost::spirit::qi::unused_type type;
	};
	
	/** function called during the actual parsing process */
	template <typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parse(Iterator & first, const Iterator & last, Context & /* context */, const Skipper & skipper, Attribute & /* attr */) const {
		if (first == last) return false;
		return skipGap(first, last, skipper);
	}
	
	/** called during error handling to create a human readable string for the error context */
	template <typename Context>
	boost::spirit::info what(Context & /* context */) const {
		return boost::spirit::info("gap");
	}
private:
	/** silence MSVC warning C4512: assignment operator could not be generated */
	GapPrimitive & operator= (const GapPrimitive &);
};


/** implementation of the parser component */
template <typename Subject>
struct gapParamPrimitive : boost::spirit::qi::primitive_parser<gapParamPrimitive<Subject> > {
	Subject subject;

	/** define the attribute type exposed by this parser component */
	template <typename Context, typename Iterator>
	struct attribute {
		typedef boost::spirit::qi::unused_type type;
	};
	
	gapParamPrimitive(const Subject & aSubject) :
		subject(aSubject)
	{}
	
	/** function called during the actual parsing process */
	template <typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parse(Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr) const {
		if (first == last) return false;
		return subject.parse(first, last, context, boost::spirit::qi::detail::unused_skipper<Skipper>(skipper), attr);
	}
	
	/** called during error handling to create a human readable string for the error context */
	template <typename Context>
	boost::spirit::info what(Context & /* context */) const {
		return boost::spirit::info("gap");
	}
private:
	/** silence MSVC warning C4512: assignment operator could not be generated */
	gapParamPrimitive & operator= (const gapParamPrimitive &);
};



} /* namespace detail */
} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


namespace boost {
namespace spirit {
namespace qi {


/** instantiation of the parser */
template <typename Modifiers>
struct make_primitive<pcf::parser::spirit::tag::gap, Modifiers> {
	typedef pcf::parser::spirit::detail::GapPrimitive result_type;
	
	/** this is the factory function object invoked in order to create an instance of the GapPrimitive */
	result_type operator() (unused_type, unused_type) const {
		return result_type();
	}
};


/** instantiation of the parser */
template <typename Modifiers, typename A0>
struct make_primitive<terminal_ex<pcf::parser::spirit::tag::gap, fusion::vector1<A0> >, Modifiers> {
	typedef typename
		result_of::compile<qi::domain, A0, Modifiers>::type
	gap_type;
	
	typedef pcf::parser::spirit::detail::gapParamPrimitive<gap_type> result_type;
	
	/** this is the factory function object invoked in order to create an instance of the gapParamPrimitive */
	template <typename Terminal>
	result_type operator() (const Terminal & term, const Modifiers & modifiers) const {
		return result_type(compile<qi::domain>(fusion::at_c<0>(term.args), modifiers));
	}
};


} /* namespace qi */
} /* namespace spirit */
} /* namespace boost */


#endif /* __LIBPCFXX_PARSER_SPIRIT_GAP_HPP__ */
