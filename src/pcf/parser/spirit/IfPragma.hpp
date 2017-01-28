/**
 * @file IfPragma.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-11-27
 * @version 2016-11-20
 * @brief Enables if pragma statements with ifPragma(if, expr, then, [elseif], [else], [block-skipper], end)[block]
 * and expectIfPragma(if, expr, then, [elseif], [else], [block-skipper], end)[block].
 * @remarks Extension to the Boost Spirit2 parser framework.
 * @remarks Use boost::spirit::unused to disable optional parameters.
 */
#ifndef __LIBPCFXX_PARSER_SPIRIT_IFPRAGMA_HPP__
#define __LIBPCFXX_PARSER_SPIRIT_IFPRAGMA_HPP__


#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <libpcfxx.hpp>
#include <pcf/parser/spirit/detail/ConditionalParse.hpp>


namespace pcf {
namespace parser {
namespace spirit {


/* define the placeholder representing our new component when building parser expressions */
BOOST_SPIRIT_TERMINAL_EX(ifPragmaVectorArg)


} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


namespace boost {
namespace spirit {


/** enables ifPragmaVectorArg(vector8<if, expr, [then], [elseif], [else], [block-skipper], end, expect>)[block] as directive */
template <typename A0>
struct use_directive<qi::domain, terminal_ex<pcf::parser::spirit::tag::ifPragmaVectorArg, fusion::vector1<A0> > >
	: mpl::true_
	/* the following makes debugging too hard (type errors are not caught in the right context) */
	/*mpl::and_<
		mpl::and_<
			mpl::or_<traits::matches<fusion::result_of::at_c<A0, 0>, qi::domain>, is_same<fusion::result_of::at_c<A0, 0>, unused_type> >,
			mpl::or_<traits::matches<fusion::result_of::at_c<A0, 1>, qi::domain>, is_same<fusion::result_of::at_c<A0, 1>, unused_type> >,
			mpl::or_<traits::matches<fusion::result_of::at_c<A0, 2>, qi::domain>, is_same<fusion::result_of::at_c<A0, 2>, unused_type> >,
			mpl::or_<traits::matches<fusion::result_of::at_c<A0, 3>, qi::domain>, is_same<fusion::result_of::at_c<A0, 3>, unused_type> >,
			mpl::or_<traits::matches<fusion::result_of::at_c<A0, 4>, qi::domain>, is_same<fusion::result_of::at_c<A0, 4>, unused_type> >
		>,  mpl::or_<traits::matches<fusion::result_of::at_c<A0, 5>, qi::domain>, is_same<fusion::result_of::at_c<A0, 5>, unused_type> >,
			mpl::or_<traits::matches<fusion::result_of::at_c<A0, 6>, qi::domain>, is_same<fusion::result_of::at_c<A0, 6>, unused_type> >
	>::type*/
{};


} /* namespace spirit */
} /* namespace boost */


namespace pcf {
namespace parser {
namespace spirit {
namespace detail {


#ifndef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
using pcf::parser::spirit::ifPragmaVectorArg;
#endif
using pcf::parser::spirit::ifPragmaVectorArg_type;


/** implementation of the parser component */
template <typename Subject, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct IfPragmaDirective : boost::spirit::qi::unary_parser<IfPragmaDirective<Subject, A0, A1, A2, A3, A4, A5, A6> > {
private:
	enum MatchType {
		MATCHED_IF,
		MATCHED_ELSEIF,
		MATCHED_ELSE,
		MATCHED_END,
		MATCHED_NONE
	};
public:
	Subject subject;
	typedef A0 IfType;
	typedef A1 ExprType;
	typedef A2 ThenType;
	typedef A3 ElseIfType;
	typedef A4 ElseType;
	typedef A5 BlockSkipperType;
	typedef A6 EndType;
	IfType ifParser;               /**< required */
	ExprType exprParser;           /**< required, attribute = bool */
	ThenType thenParser;           /**< optional, can be boost::spirit::unused */
	ElseIfType elseIfParser;       /**< optional, can be boost::spirit::unused */
	ElseType elseParser;           /**< optional, can be boost::spirit::unused */
	BlockSkipperType blockSkipper; /**< optional, can be boost::spirit::unused */
	EndType endParser;             /**< required */
	bool expect;                   /**< required */
	
	/** define the attribute type exposed by this parser component */
	template <typename Context, typename Iterator>
	struct attribute {
		typedef typename boost::spirit::traits::build_optional<
			typename boost::spirit::traits::attribute_of<Subject, Context, Iterator>::type
		>::type type;
	};
	
	IfPragmaDirective(
		const Subject & aSubject,
		const A0 & a0,
		const A1 & a1,
		const A2 & a2,
		const A3 & a3,
		const A4 & a4,
		const A5 & a5,
		const A6 & a6,
		const bool doExpect
	) :
		subject     (aSubject),
		ifParser    (a0),
		exprParser  (a1),
		thenParser  (a2),
		elseIfParser(a3),
		elseParser  (a4),
		blockSkipper(a5),
		endParser   (a6),
		expect      (doExpect)
	{}

	/** function called during the actual parsing process */
	template <typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parse(Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr) const {
		/* exprParser needs to return a bool */
		BOOST_MPL_ASSERT_MSG(
			(boost::is_same<typename boost::spirit::traits::attribute_of<ExprType, Context, Iterator>::type, bool>::value),
			EXPRESSION_ATTRIBUTE_TYPE_NEEDS_TO_BE_A_BOOL,
			(typename boost::spirit::traits::attribute_of<ExprType, Context, Iterator>::type)
		);
		boost::spirit::qi::skip_over(first, last, skipper);
		const Iterator savedPos(first);
		bool condition(false);
		size_t matchedStatement(0), statement(0);
		/* if */
		if ( ! ifParser.parse(first, last, context, skipper, boost::spirit::unused) ) {
			first = savedPos;
			return false;
		}
		MatchType match(MATCHED_IF);
		do {
			/* expression */
			statement++;
			if ( ! parseOrFail(exprParser, first, last, context, skipper, condition, expect) ) {
				first = savedPos;
				return false;
			}
			if (condition && matchedStatement == 0) matchedStatement = statement;
			/* then */
			if ( ! boost::is_same<boost::spirit::unused_type, ThenType>::value ) {
				if ( ! conditionalParse(thenParser, first, last, context, skipper, boost::spirit::unused, expect) ) {
					first = savedPos;
					return false;
				}
			}
			/* block (handle nested if statements here) */
			if ( ! parseBlock(first, last, context, skipper, attr, matchedStatement == statement, true, match) ) {
				first = savedPos;
				return false;
			}
			if (match == MATCHED_END) return true;
		} while (match == MATCHED_ELSEIF);
		if (match == MATCHED_ELSE) {
			if ( ! parseBlock(first, last, context, skipper, attr, matchedStatement == 0, false, match) ) {
				first = savedPos;
				return false;
			}
		}
		return match == MATCHED_END;
	}
	
	/** called during error handling to create a human readable string for the error context */
	template <typename Context>
	boost::spirit::info what(Context & context) const {
		return boost::spirit::info("if-pragma", subject.what(context));
	}
private:
	/** silence MSVC warning C4512: assignment operator could not be generated */
	IfPragmaDirective & operator= (const IfPragmaDirective &);
	
	/** @return true on success, else false */
	template <typename Parser, typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parseOrFail(const Parser & aParser, Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr, const bool isExpected) const {
		if ( ! isExpected ) {
			return aParser.parse(first, last, context, skipper, attr);
		}
		boost::spirit::qi::detail::expect_function<Iterator, Context, Skipper, boost::spirit::qi::expectation_failure<Iterator> > expectationParser(first, last, context, skipper);
		expectationParser.is_first = false;
		return ( ! expectationParser(aParser, attr) );
	}
	
	template <typename Iterator, typename Context, typename Skipper>
	bool parseNext(Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, MatchType & match, const bool isExpected) const {
		/* continuing elseif */
		if ( ! boost::is_same<boost::spirit::unused_type, ElseIfType>::value ) {
			const Iterator savedPos = first;
			if ( conditionalParse(elseIfParser, first, last, context, skipper, boost::spirit::unused, false) ) {
				match = MATCHED_ELSEIF;
				return true;
			}
			first = savedPos;
		}
		/* continuing else */
		if ( ! boost::is_same<boost::spirit::unused_type, ElseType>::value ) {
			const Iterator savedPos = first;
			if ( conditionalParse(elseParser, first, last, context, skipper, boost::spirit::unused, false) ) {
				match = MATCHED_ELSE;
				return true;
			}
			first = savedPos;
		}
		/* final end */
		if ( parseOrFail(endParser, first, last, context, skipper, boost::spirit::unused, isExpected) ) {
			match = MATCHED_END;
			return true;
		}
		return false;
	}
	
	template <typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parseBlock(Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr, boost::mpl::false_) const {
		/* create a local value if Attribute is not unused_type */
		typename boost::spirit::result_of::optional_value<Attribute>::type val = typename boost::spirit::result_of::optional_value<Attribute>::type();

		if ( subject.parse(first, last, context, skipper, val) ) {
			/* assign the parsed value into our attribute */
			boost::spirit::traits::assign_to(val, attr);
			return true;
		}
		return false;
	}

	template <typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parseBlock(Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr, boost::mpl::true_) const {
		return subject.parse(first, last, context, skipper, attr);
	}
	
	template <typename Iterator, typename Context, typename Skipper, typename Attribute>
	bool parseBlock(Iterator & first, const Iterator & last, Context & context, const Skipper & skipper, Attribute & attr, const bool condition, const bool allowElse, MatchType & match) const {
		if ( condition ) {
			/* block */
			typedef typename boost::spirit::result_of::optional_value<Attribute>::type attribute_type;
			if ( ! parseBlock(first, last, context, skipper, attr, boost::spirit::traits::is_container<attribute_type>()) ) {
				return false;
			}
			/* find next */
			match = MATCHED_NONE;
			if ( parseNext(first, last, context, skipper, match, expect) ) {
				if (match == MATCHED_ELSEIF || match == MATCHED_ELSE) {
					return allowElse; /* allowElse == false means we found a stay elseif or else */
				} else if (match == MATCHED_END) {
					return true;
				}
			}
			return false;
		}
		size_t depth(0);
		while (first != last) {
			Iterator savedPos;
			/* nested if */
			if ( ifParser.parse(first, last, context, skipper, boost::spirit::unused) ) {
				depth++;
			} else {
				if (depth < 1) {
					savedPos = first;
					match = MATCHED_NONE;
					if ( parseNext(first, last, context, skipper, match, false) ) {
						if (match == MATCHED_ELSEIF || match == MATCHED_ELSE) {
							return allowElse; /* allowElse == false means we found a stay elseif or else */
						} else if (match == MATCHED_END) {
							return true;
						}
					}
					first = savedPos;
				} else { /* nested */
					/* end */
					savedPos = first;
					if ( endParser.parse(first, last, context, skipper, boost::spirit::unused) ) {
						depth--;
						continue; /* keep block skipper from consuming following ends */
					} else {
						first = savedPos;
					}
				}
				/* skip until end of block */
				if ( boost::is_same<boost::spirit::unused_type, BlockSkipperType>::value ) {
					++first;
				} else {
					savedPos = first;
					if ( ! conditionalParse(blockSkipper, first, last, context, skipper, boost::spirit::unused, expect) ) {
						first = savedPos;
						break;
					}
				}
			}
		}
		return false; /* reached end of input without finding end of block */
	}
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
	terminal_ex<pcf::parser::spirit::tag::ifPragmaVectorArg, fusion::vector1<A0> >,
	Subject,
	Modifiers
> {
	typedef typename fusion::result_of::value_at_c<A0, 0>::type IfType;
	typedef typename fusion::result_of::value_at_c<A0, 1>::type ExprType;
	typedef typename fusion::result_of::value_at_c<A0, 2>::type ThenType;
	typedef typename fusion::result_of::value_at_c<A0, 3>::type ElseIfType;
	typedef typename fusion::result_of::value_at_c<A0, 4>::type ElseType;
	typedef typename fusion::result_of::value_at_c<A0, 5>::type BlockSkipperType;
	typedef typename fusion::result_of::value_at_c<A0, 6>::type EndType;
	typedef typename boost::remove_const<
		typename fusion::result_of::value_at_c<A0, 7>::type
	>::type ExpectType;
	
	/* IfType needs to be a Boost Spirit parser */
	BOOST_MPL_ASSERT_MSG(
		(traits::matches<qi::domain, IfType>::value),
		INVALID_SPIRIT_QI_PARSER_FOR_IF,
		(IfType)
	);
	/* ExprType needs to be a Boost Spirit parser */
	BOOST_MPL_ASSERT_MSG(
		(traits::matches<qi::domain, ExprType>::value),
		INVALID_SPIRIT_QI_PARSER_FOR_EXPRESSION,
		(ExprType)
	);
	/* ThenType needs to be a Boost Spirit parser or unused */
	BOOST_MPL_ASSERT_MSG(
		(mpl::or_<traits::matches<qi::domain, ThenType>, boost::is_same<unused_type, ThenType> >::value),
		INVALID_SPIRIT_QI_PARSER_FOR_THEN,
		(ThenType)
	);
	/* ElseIfType needs to be a Boost Spirit parser or unused */
	BOOST_MPL_ASSERT_MSG(
		(mpl::or_<traits::matches<qi::domain, ElseIfType>, boost::is_same<unused_type, ElseIfType> >::value),
		INVALID_SPIRIT_QI_PARSER_FOR_ELSEIF,
		(ElseIfType)
	);
	/* ElseType needs to be a Boost Spirit parser or unused */
	BOOST_MPL_ASSERT_MSG(
		(mpl::or_<traits::matches<qi::domain, ElseType>, boost::is_same<unused_type, ElseType> >::value),
		INVALID_SPIRIT_QI_PARSER_FOR_ELSE,
		(ElseType)
	);
	/* BlockSkipperType needs to be a Boost Spirit parser */
	BOOST_MPL_ASSERT_MSG(
		(mpl::or_<traits::matches<qi::domain, BlockSkipperType>, boost::is_same<unused_type, BlockSkipperType> >::value),
		INVALID_SPIRIT_QI_PARSER_FOR_BLOCK_SKIPPER,
		(BlockSkipperType)
	);
	/* EndType needs to be a Boost Spirit parser */
	BOOST_MPL_ASSERT_MSG(
		(traits::matches<qi::domain, EndType>::value),
		INVALID_SPIRIT_QI_PARSER_FOR_END,
		(EndType)
	);
	/* ExpectType needs to be a bool */
	BOOST_MPL_ASSERT_MSG(
		(boost::is_same<bool, ExpectType>::value),
		PARAM_EXPECT_NEEDS_BOOL_TYPE,
		(ExpectType)
	);
	
	typedef typename result_of::compile<qi::domain, IfType, Modifiers>::type ResultIfType;
	
	typedef typename result_of::compile<qi::domain, ExprType, Modifiers>::type ResultExprType;
	
	typedef typename mpl::if_<
		boost::is_same<unused_type, ThenType>,
		unused_type,
		typename result_of::compile<qi::domain, ThenType, Modifiers>::type
	>::type ResultThenType;
	
	typedef typename mpl::if_<
		boost::is_same<unused_type, ElseIfType>,
		unused_type,
		typename result_of::compile<qi::domain, ElseIfType, Modifiers>::type
	>::type ResultElseIfType;
	
	typedef typename mpl::if_<
		boost::is_same<unused_type, ElseType>,
		unused_type,
		typename result_of::compile<qi::domain, ElseType, Modifiers>::type
	>::type ResultElseType;
	
	typedef typename mpl::if_<
		boost::is_same<unused_type, BlockSkipperType>,
		unused_type,
		typename result_of::compile<qi::domain, BlockSkipperType, Modifiers>::type
	>::type ResultBlockSkipperType;
	
	typedef typename result_of::compile<qi::domain, EndType, Modifiers>::type ResultEndType;
	
	typedef typename pcf::parser::spirit::detail::IfPragmaDirective<
		Subject,
		ResultIfType,
		ResultExprType,
		ResultThenType,
		ResultElseIfType,
		ResultElseType,
		ResultBlockSkipperType,
		ResultEndType
	> result_type;
	
	/** this is the factory function object invoked in order to create an instance of the IfPragmaDirective */
	template <typename Terminal>
	result_type operator() (const Terminal & term, const Subject & subject, const Modifiers & modifiers) const {
		return result_type(
			subject,
			compile<qi::domain>(fusion::at_c<0>(fusion::at_c<0>(term.args)), modifiers),
			compile<qi::domain>(fusion::at_c<1>(fusion::at_c<0>(term.args)), modifiers),
			compile<qi::domain>(fusion::at_c<2>(fusion::at_c<0>(term.args)), modifiers),
			compile<qi::domain>(fusion::at_c<3>(fusion::at_c<0>(term.args)), modifiers),
			compile<qi::domain>(fusion::at_c<4>(fusion::at_c<0>(term.args)), modifiers),
			compile<qi::domain>(fusion::at_c<5>(fusion::at_c<0>(term.args)), modifiers),
			compile<qi::domain>(fusion::at_c<6>(fusion::at_c<0>(term.args)), modifiers),
			fusion::at_c<7>(fusion::at_c<0>(term.args))
		);
	}
};


} /* namespace qi */
} /* namespace spirit */
} /* namespace boost */


namespace boost {
namespace spirit {
namespace traits {


template <typename Subject, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct has_semantic_action<pcf::parser::spirit::detail::IfPragmaDirective<Subject, A0, A1, A2, A3, A4, A5, A6> >
	: unary_has_semantic_action<Subject>
{};


template <typename Subject, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename Attribute, typename Context, typename Iterator>
struct handles_container<pcf::parser::spirit::detail::IfPragmaDirective<Subject, A0, A1, A2, A3, A4, A5, A6>, Attribute, Context, Iterator>
	: unary_handles_container<Subject, Attribute, Context, Iterator>
{};


} /* namespace traits */
} /* namespace spirit */
} /* namespace boost */


namespace pcf {
namespace parser {
namespace spirit {


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a ifPragma terminal.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, boost::spirit::unused_type, boost::spirit::unused_type, boost::spirit::unused_type, A3, bool> >::type
ifPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, boost::spirit::unused, boost::spirit::unused, boost::spirit::unused, a3, false));
}


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a ifPragma terminal.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - ELSE parser
 * @param[in] a4 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3, typename A4>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, boost::spirit::unused_type, A3, boost::spirit::unused_type, A4, bool> >::type
ifPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, boost::spirit::unused, a3, boost::spirit::unused, a4, false));
}


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a ifPragma terminal.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - ELSEIF parser
 * @param[in] a4 - ELSE parser
 * @param[in] a5 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, A3, A4, boost::spirit::unused_type, A5, bool> >::type
ifPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, a3, a4, boost::spirit::unused, a5, false));
}


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a ifPragma terminal.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - ELSEIF parser
 * @param[in] a4 - ELSE parser
 * @param[in] a5 - BLOCK skipper
 * @param[in] a6 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, A3, A4, A5, A6, bool> >::type
ifPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5, const A6 & a6) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, a3, a4, a5, a6, false));
}


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a expectIfPragma terminal
 * which expects a certain syntax and throws an expectation error else.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, boost::spirit::unused_type, boost::spirit::unused_type, boost::spirit::unused_type, A3, bool> >::type
expectIfPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, boost::spirit::unused, boost::spirit::unused, boost::spirit::unused, a3, true));
}


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a expectIfPragma terminal
 * which expects a certain syntax and throws an expectation error else.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - ELSE parser
 * @param[in] a4 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3, typename A4>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, boost::spirit::unused_type, A3, boost::spirit::unused_type, A4, bool> >::type
expectIfPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, boost::spirit::unused, a3, boost::spirit::unused, a4, true));
}


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a expectIfPragma terminal
 * which expects a certain syntax and throws an expectation error else.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - ELSEIF parser
 * @param[in] a4 - ELSE parser
 * @param[in] a5 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, A3, A4, boost::spirit::unused_type, A5, bool> >::type
expectIfPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, a3, a4, boost::spirit::unused, a5, true));
}


/**
 * Helper function to work around Boost Spirit's three arguments limit. Creates a expectIfPragma terminal
 * which expects a certain syntax and throws an expectation error else.
 * 
 * @param[in] a0 - IF parser
 * @param[in] a1 - EXPRESSION parser
 * @param[in] a2 - THEN parser
 * @param[in] a3 - ELSEIF parser
 * @param[in] a4 - ELSE parser
 * @param[in] a5 - BLOCK skipper
 * @param[in] a6 - END parser
 * @return Boost Spirit terminal/directive
 */
template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline typename boost::spirit::terminal<tag::ifPragmaVectorArg>::result<boost::fusion::vector8<A0, A1, A2, A3, A4, A5, A6, bool> >::type
expectIfPragma(const A0 & a0, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5, const A6 & a6) {
	return ifPragmaVectorArg(boost::fusion::make_vector(a0, a1, a2, a3, a4, a5, a6, true));
}


} /* namespace spirit */
} /* namespace parser */
} /* namespace pcf */


#endif /* __LIBPCFXX_PARSER_SPIRIT_IFPRAGMA_HPP__ */
