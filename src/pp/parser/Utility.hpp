/**
 * @file Utility.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-10-31
 * @version 2016-05-01
 */
#ifndef __PP_PARSER_UTILITY_HPP__
#define __PP_PARSER_UTILITY_HPP__


#include <boost/config/warning_disable.hpp>
#include <boost/foreach.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/path/Utility.hpp>
#include "../Type.hpp"


namespace pp {
namespace parser {


namespace fusion = boost::fusion;
namespace phx = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace encoding = boost::spirit::standard;


namespace {


/**
 * Boost Spirit Qi matching grammar for character escaping.
 */
struct EscapedChar : qi::symbols<char, char> {
	/** Default constructor. */
	EscapedChar() {
		add
			("a",  '\a')
			("b",  '\b')
			("f",  '\f')
			("n",  '\n')
			("r",  '\r')
			("t",  '\t')
			("v",  '\v')
		;
	}
} escapedChar; /**< Instantiated grammar for direct usage. */


/**
 * Implementation of a Boost Phoenix function to return the tag member of a variable.
 */
struct getTagImpl {
	template <typename Sig>
	struct result;
	
	template <typename This, typename Arg>
	struct result<This(const Arg &)> {
		typedef std::string type;
	};

	template <typename Arg>
	std::string operator()(const Arg & what_) const {
		return what_.tag;
	}
};


/**
 * Instantiation of getTagImpl for direct usage as Boost Phoenix actor.
 * 
 * Example:@n
 * @code{.cpp}
 * tag = getTag(var)(); // same as tag = var.tag
 * @endcode
 */
static phx::function<getTagImpl> getTag;


/**
 * Boost Spirit Qi matching grammar for verbosity string parsing.
 */
struct VerbosityValue : qi::symbols<char, pp::Verbosity> {
	/** Default constructor. */
	VerbosityValue() {
		add
			("ERROR", pp::VERBOSITY_ERROR)
			("WARN",  pp::VERBOSITY_WARN)
			("INFO",  pp::VERBOSITY_INFO)
			("DEBUG", pp::VERBOSITY_DEBUG)
		;
		name("ERROR, WARN, INFO or DEBUG");
	}
} verbosityValue; /**< Instantiated grammar for direct usage. */


/**
 * Boost Spirit Qi matching grammar for verbosity command string parsing.
 */
struct VerbosityCommand : qi::symbols<char, pp::Verbosity> {
	/** Default constructor. */
	VerbosityCommand() {
		add
			("error", pp::VERBOSITY_ERROR)
			("warn",  pp::VERBOSITY_WARN)
			("info",  pp::VERBOSITY_INFO)
			("debug", pp::VERBOSITY_DEBUG)
		;
		name("error, warn, info or debug");
	}
} verbosityCommand; /**< Instantiated grammar for direct usage. */


} /* anonymous namespace */


void throwRegexException(const pp::StringLiteral & regexStr, const boost::regex_error & e);
boost::optional<ssize_t> parseVariableIndex(const std::string & input, const std::string & variable);


} /* namespace parser */
} /* namespace pp */


#endif /* __PP_PARSER_UTILITY_HPP__ */
