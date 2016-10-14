/**
 * @file Name.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-11-25
 * @version 2016-05-01
 * @brief Enables expression naming by string as name(string)[p] for easier expectation handling.
 * @remarks Extension to the Boost Spirit2 parser framework.
 */
#ifndef __LIBPCFXX_PARSER_SPIRIT_NAME_HPP__
#define __LIBPCFXX_PARSER_SPIRIT_NAME_HPP__


#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <libpcfxx.hpp>


namespace pcf {
namespace parser {
namespace spirit {


/* define the placeholder representing our new component when building parser expressions */
BOOST_SPIRIT_TERMINAL_EX(name)


} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


namespace boost {
namespace spirit {


/** enables name(string)[p] as directive only and only for parser expressions (qi::domain) */
template <typename String>
struct use_directive<qi::domain, terminal_ex<pcf::parser::spirit::tag::name, fusion::vector1<String> > >
	: traits::is_string<String>::type
{};


/** enables *lazy* name(string)[p] as directive only and only for parser expressions (qi::domain)
 *  @remarks the directive will be encapsulated by a lazy_directive which returns "lazy-directive"
 *  name */
template <>
struct use_lazy_directive<qi::domain, pcf::parser::spirit::tag::name, 1 /* arity */>
	: mpl::true_
{};


} /* namespace spirit */
} /* namespace boost */


namespace pcf {
namespace parser {
namespace spirit {
namespace detail {


#ifndef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
using pcf::parser::spirit::name;
#endif
using pcf::parser::spirit::name_type;


/** implementation of the parser component */
template <typename Subject, typename String>
struct NameDirective : boost::spirit::qi::unary_parser<NameDirective<Subject, String> > {
	typedef typename
		boost::remove_const<typename boost::spirit::traits::char_type_of<String>::type>::type
	char_type;
	
	typedef std::basic_string<char_type> NameStringType;
	
	Subject subject;
	NameStringType exposedName;

	/** define the attribute type exposed by this parser component */
	template <typename Context, typename Iterator>
	struct attribute {
		typedef typename boost::spirit::traits::attribute_of<Subject, Context, Iterator>::type type;
	};
	
	NameDirective(const Subject & aSubject, typename boost::add_reference<typename boost::add_const<String>::type>::type aName) :
		subject(aSubject),
		exposedName(aName)
	{}

	/** function called during the actual parsing process (propagate subject grammar here) */
	template <typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parse(Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr) const {
		return subject.parse(first, last, context, skipper, attr);
	}

	/** called during error handling to create a human readable string for the error context */
	template <typename Context>
	boost::spirit::info what(Context & /* context */) const {
		return boost::spirit::info(this->exposedName);
	}
private:
	/** silence MSVC warning C4512: assignment operator could not be generated */
	NameDirective & operator= (const NameDirective &);
};


} /* namespace detail */
} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


namespace boost {
namespace spirit {
namespace qi {


/** instantiation of the parser */
template <typename A0, typename Subject, typename Modifiers>
struct make_directive<
	terminal_ex<pcf::parser::spirit::tag::name, fusion::vector1<A0> >,
	Subject,
	Modifiers
> {
	typedef typename boost::add_const<A0>::type ConstString;
	typedef pcf::parser::spirit::detail::NameDirective<Subject, ConstString> result_type;
	
	/** this is the factory function object invoked in order to create an instance of the NameDirective */
	template <typename Terminal>
	result_type operator ()(const Terminal & term, const Subject & subject, unused_type) const {
		return result_type(subject, fusion::at_c<0>(term.args));
	}
};


} /* namespace qi */
} /* namespace spirit */
} /* namespace boost */


namespace boost {
namespace spirit {
namespace traits {


template <typename Subject, typename String>
struct has_semantic_action<pcf::parser::spirit::detail::NameDirective<Subject, String> >
	: unary_has_semantic_action<Subject>
{};


template <typename Subject, typename String, typename Attribute, typename Context, typename Iterator>
struct handles_container<pcf::parser::spirit::detail::NameDirective<Subject, String>, Attribute, Context, Iterator>
	: unary_handles_container<Subject, Attribute, Context, Iterator>
{};


} /* namespace traits */
} /* namespace spirit */
} /* namespace boost */


#endif /* __LIBPCFXX_PARSER_SPIRIT_NAME_HPP__ */
