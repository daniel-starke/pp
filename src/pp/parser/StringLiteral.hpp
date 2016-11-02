/**
 * @file StringLiteral.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-10-31
 * @version 2016-05-01
 */
#ifndef __PP_PARSER_STRINGLITERAL_HPP__
#define __PP_PARSER_STRINGLITERAL_HPP__


#include <boost/config/warning_disable.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/path/Utility.hpp>
#include "../Variable.hpp"
#include "Utility.hpp"


#ifndef DOXYGEN_SHOULD_SKIP_THIS
/* adapt pp::StringLiteralPart to Boost Fusion */
BOOST_FUSION_ADAPT_STRUCT(
	pp::StringLiteralPart,
	(std::string, value)
	(pp::StringLiteralPart::Type, type)
	(pp::StringLiteralFunctionVector, functions)
)
#endif


namespace pp {
namespace parser {


namespace fusion = boost::fusion;
namespace phx = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace encoding = boost::spirit::standard;


/**
 * String literal grammar of the parallel processor script.
 *
 * @tparam Iterator - multi-pass forward iterator type
 */
template <typename Iterator>
struct StringLiteral : qi::grammar<Iterator, StringLiteralCaptureVector(pp::StringLiteral::ParsingFlags, boost::optional<char>)> {
	pp::StringLiteral::ParsingFlags currentParsingFlags; /**< Currently enforced parsing flags. */
	boost::optional<char> endOfInputCharacter; /**< Optionally terminate grammar at this character. */
	bool printError; /**< Set to true to print parsing errors to the standard output console. Throw an exception otherwise. */
	/* simple grammars */
	qi::rule<Iterator> endOfInput;
	qi::rule<Iterator> startReplacement;
	qi::rule<Iterator> beginVarGroup;
	qi::rule<Iterator> endVarGroup;
	qi::rule<Iterator> beginCaptureGroup;
	qi::rule<Iterator> endCaptureGroup;
	qi::rule<Iterator> beginIgnoreCaptureGroup;
	qi::rule<Iterator> beginNamedCaptureGroup;
	qi::rule<Iterator> endNamedCaptureGroup;
	qi::rule<Iterator> subStringParamDelimiter;
	qi::rule<Iterator, char()> beginVarIdxGroup;
	qi::rule<Iterator, char()> endVarIdxGroup;
	qi::uint_parser<char, 16, 2, 2> hexOctet;
	qi::rule<Iterator, char()> basicChar;
	/* complex grammars */
	qi::rule<Iterator, pp::StringLiteralPart()> stringPart;
	qi::rule<Iterator, StringLiteralReplacementPair(), qi::locals<char, Iterator> > replacementPattern;
	qi::rule<
		Iterator,
		pp::StringLiteralFunctionPair(bool),
		qi::locals<Iterator /* _a */, std::string /* _b */, StringLiteralReplacementPair /* _c */>
	> function;
	qi::rule<Iterator, pp::StringLiteralPart()> variable;
	qi::rule<Iterator, pp::StringLiteralPart()> variablePart;
	qi::rule<Iterator, pp::StringLiteralList()> partList;
	qi::rule<Iterator, pp::StringLiteralCapturePair(), qi::locals<pp::CaptureNameVector> > capture;
	qi::rule<Iterator, pp::StringLiteralCaptureVector()> captureList;
	qi::rule<Iterator, pp::StringLiteralCaptureVector(pp::StringLiteral::ParsingFlags, boost::optional<char>)> startRule;
	size_t captureIndex;
	
	/**
	 * Constructor.
	 *
	 * @param[in] parsingFlags - flags for string literal parsing
	 * @param[in] eoic - optional end of input character
	 * @param[in] pe - true to print errors to the standard output console, false to throw an exception instead
	 */
	StringLiteral(const pp::StringLiteral::ParsingFlags parsingFlags = pp::StringLiteral::STANDARD, const boost::optional<char> & eoic = boost::optional<char>(), const bool pe = false):
		StringLiteral::base_type(startRule, "string literal"),
		currentParsingFlags(parsingFlags),
		endOfInputCharacter(eoic),
		printError(pe)
	{
		/* use these Spirit terminals and Phoenix actors */
		using boost::spirit::repository::qi::iter_pos;
		using encoding::char_;
		using encoding::string;
		using phx::at_c;
		using phx::construct;
		using qi::_1;
		using qi::_2;
		using qi::_a;
		using qi::_b;
		using qi::_c;
		using qi::_r1;
		using qi::_val;
		using qi::as_string;
		using qi::int_;
		using qi::lit;
		using qi::eps;
		using qi::omit;
		/* non-changing rules */
		stringPart.name("string");
		replacementPattern.name("replacement pattern");
		function.name("function");
		variable.name("variable");
		variablePart.name("{variable}");
		partList.name("string|{variable}");
		capture.name("capture group");
		captureList.name("capture list");
		
		/* normal string literal omitting their match */
#define MAKE_LITERAL(var, x, y) \
		var.name(x); \
		var = lit(y);
		
		MAKE_LITERAL(startReplacement, "$", '$');
		MAKE_LITERAL(beginVarGroup, "{", '{');
		MAKE_LITERAL(endVarGroup, "}", '}');
		MAKE_LITERAL(beginCaptureGroup, "(", '(');
		MAKE_LITERAL(endCaptureGroup, ")", ')');
		MAKE_LITERAL(beginIgnoreCaptureGroup, "?:", "?:");
		MAKE_LITERAL(beginNamedCaptureGroup, "?<", "?<");
		MAKE_LITERAL(endNamedCaptureGroup, ">", '>');
		MAKE_LITERAL(subStringParamDelimiter, ",", ',');
		
#undef MAKE_LITERAL
		
		/* normal string literals that return their match */
#define MAKE_LITERAL(var, x, y) \
		var.name(x); \
		var = char_(y);
		
		MAKE_LITERAL(beginVarIdxGroup, "[", '[');
		MAKE_LITERAL(endVarIdxGroup, "]", ']');
		
#undef MAKE_LITERAL
		
		replacementPattern = (
			&char_[_a = _1]
				>> omit[iter_pos[_b = _1]] >> as_string[
					omit[char_(_a)] >> (*(!char_(_a) >> char_)) >> omit[char_(_a)]
				][at_c<0>(_val) = phx::bind<pp::StringLiteral>(&StringLiteral::getRawStringLiteral, _1, _b)]
				>> omit[iter_pos[_b = _1]] >> as_string[
					(*(!char_(_a) >> char_)) >> omit[char_(_a)]
				][at_c<1>(_val) = phx::bind<pp::StringLiteral>(&StringLiteral::getRawStringLiteral, _1, _b)]
		);
		
		/* @param[in] _r1 - true for extended functions of non-dynamic variables */
		function = ( /* dynamic variable */
			  string("win")          [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionWin)]
			| string("unix")         [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionUnix)]
#if defined(PCF_IS_WIN)
			| string("native")       [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionWin)]
#else /* not Windows */
			| string("native")       [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionUnix)]
#endif /* Windows */
			| string("esc")          [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionEsc)]
			| string("upper")        [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionUpper)]
			| string("lower")        [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionLower)]
			| &startReplacement >> (
				omit[iter_pos[_a = _1] >> startReplacement] >> replacementPattern[_c = _1] >> omit[iter_pos[_b = construct<std::string>(_a, _1)]]
			)[_val = phx::bind<StringLiteralFunctionPair>(&StringLiteral::getReplacementFunction, _b, _c)]
			| &int_ >> (
				omit[iter_pos[_a = _1]] >> int_ >> -(subStringParamDelimiter >> int_) >> omit[iter_pos[_b = construct<std::string>(_a, _1)]]
			)[_val = phx::bind<StringLiteralFunctionPair>(&StringLiteral::getSubstrFunction, _b, _1, _2)]
			| (eps(_r1) >> ( /* non-dynamic variable */
				  string("directory")[_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionDirectory)]
				| string("filename") [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionFilename)]
				| string("file")     [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionFile)]
				| string("extension")[_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionExtension)]
				| string("exists")   [_val = construct<StringLiteralFunctionPair>(_1, pp::StringLiteral::functionExists)]
			))
		);
		
		this->initGrammar(parsingFlags, eoic, true);
	}

private:
	/**
	 * Initialize the parsing grammar by the given options.
	 *
	 * @param[in] parsingFlags - flags for string literal parsing
	 * @param[in] eoic - optional end of input character
	 * @param[in] forceReInit - true to re-initialize the grammar, false to leave it as it is currently
	 */
	void initGrammar(const pp::StringLiteral::ParsingFlags parsingFlags, const boost::optional<char> & eoic, const bool forceReInit = false) {
		if (this->currentParsingFlags == parsingFlags && this->endOfInputCharacter == eoic && ( ! forceReInit )) return; /* no change */
		using encoding::char_;
		using encoding::string;
		using phx::at_c;
		using phx::clear;
		using phx::construct;
		using phx::push_back;
		using phx::size;
		using phx::val;
		using qi::_a;
		using qi::_r1;
		using qi::_r2;
		using qi::_val;
		using qi::_1;
		using qi::_2;
		using qi::_3;
		using qi::_4;
		using qi::as_string;
		using qi::eps;
		using qi::fail;
		using qi::lit;
		using qi::omit;
		using qi::on_error;
		
		this->currentParsingFlags = parsingFlags;
		this->endOfInputCharacter = eoic;
		
		if ( this->endOfInputCharacter ) {
			endOfInput = omit[char_(*endOfInputCharacter) | qi::eoi];
		} else {
			endOfInput = qi::eoi;
		}
		
		if (parsingFlags == (pp::StringLiteral::RAW | pp::StringLiteral::NO_ESCAPE)) {
			basicChar = !endOfInput >> char_;
		} else {
			basicChar = !endOfInput >> (
				(lit('\\') >> ((lit('x') >> hexOctet) | escapedChar | char_)) | char_
			);
		}
		
		if (parsingFlags == pp::StringLiteral::RAW) {
			stringPart = !endOfInput >> as_string[+basicChar][_val = construct<pp::StringLiteralPart>(_1, val(pp::StringLiteralPart::STRING))];
			
			partList = !endOfInput >> +stringPart;
		} else {
			if (parsingFlags == pp::StringLiteral::ENABLE_CAPTURES) {
				stringPart = !endOfInput >> (
					(
						as_string[+(!omit[char_("{()")] >> basicChar)]
					)[_val = construct<pp::StringLiteralPart>(_1, val(pp::StringLiteralPart::STRING))]
				);
			} else {
				stringPart = !endOfInput >> (
					(
						as_string[+(!omit[char_("{")] >> basicChar)]
					)[_val = construct<pp::StringLiteralPart>(_1, val(pp::StringLiteralPart::STRING))]
				);
			}
			
			variable = !endOfInput >> (
				eps[at_c<1>(_val) = val(pp::StringLiteralPart::VARIABLE)] >> eps[clear(at_c<2>(_val))] >> (
					as_string[
						( /* destination/dependency variable with index */
							(string("destination") | string("dependency"))
							>> -(
								beginVarIdxGroup
								> (char_("0") | (char_("1-9") >> *char_("0-9")))
								> endVarIdxGroup
							)
						)
						| (char_("_a-zA-Z") >> *char_("_a-zA-Z0-9")) /* variable name */
						| char_("?")
						| char_("0") /* whole capture */
						| (char_("1-9") >> *char_("0-9")) /* capture index */
					][at_c<0>(_val) = _1] >> *(lit(":") > function(true)[push_back(at_c<2>(_val), _1)])
				) 
				| (
					as_string[string("*") | string("@*")][at_c<0>(_val) = _1] /* dynamic variable */
					>> *(lit(":") > function(false)[push_back(at_c<2>(_val), _1)])
				)
			);
			
			variablePart = !endOfInput >> (
				beginVarGroup
				> variable
				> endVarGroup
			);
			
			partList = !endOfInput >> (
				+(variablePart | stringPart)
			);
		}
		
		if (parsingFlags == pp::StringLiteral::ENABLE_CAPTURES) {
			capture = !endOfInput >> (
				eps[phx::bind(&StringLiteral::setCaptureIndex, this, _a)]
				>> (
					  (partList >> eps[clear(_a)])
					| (beginCaptureGroup >> !(beginIgnoreCaptureGroup | beginNamedCaptureGroup) >> partList > endCaptureGroup >> eps[++phx::ref(captureIndex)])
					| (beginCaptureGroup >> beginIgnoreCaptureGroup >> partList > endCaptureGroup >> eps[clear(_a)])
					| (
						beginCaptureGroup >> beginNamedCaptureGroup >> omit[as_string[char_("_a-zA-Z") >> *char_("_a-zA-Z0-9")][push_back(_a, _1)]] > endNamedCaptureGroup
						>> partList
						 > endCaptureGroup
						>> eps[++phx::ref(captureIndex)]
					)
				)[_val = construct<pp::StringLiteralCapturePair>(_a, _1)]
			);
			
			this->captureIndex = 1;
			captureList = !endOfInput >> (
				/* this does not work... compiler bug? */
				/* eps[phx::ref(this->captureIndex) = val(1)] >> *capture */
				eps >> *capture
			);
		} else if (parsingFlags == (pp::StringLiteral::STANDARD | pp::StringLiteral::RAW)) {
			capture = !endOfInput >> (
				eps[clear(_a)]
				>> partList[_val = construct<pp::StringLiteralCapturePair>(_a, _1)]
			);
			
			captureList = *capture;
		}
		
		startRule %= (
			eps[phx::bind(&StringLiteral::initGrammar, this, _r1, _r2, false)] >> !endOfInput >> captureList
		);
		
		if ( this->printError ) {
			on_error<fail>(
				startRule,
				phx::ref(std::cout) << "Error: Failed to parse string literal." << std::endl
				<< construct<std::string>(_1, _2) << std::endl
				<< construct<std::string>(size(construct<std::string>(_1, _3)), ' ') << "^- expected '" << getTag(_4) << "' here" << std::endl
			);
		}
	}
	
	/**
	 * Converts the given string to a string literal as it.
	 * 
	 * @param[in] str - convert this string
	 * @param[in] it - extract script location from this iterator
	 * @remarks needs to be specialized for different iterator types by partial template specialization
	 */
	static pp::StringLiteral getRawStringLiteral(const std::string & str, const Iterator & it) {
		const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
		return pp::StringLiteral(str, LineInfo(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column)), pp::StringLiteral::RAW);
	}

	/**
	 * Sets the current capture index to the given output variable.
	 * 
	 * @param[in,out] var - set capture index here
	 */
	void setCaptureIndex(CaptureNameVector & var) const {
		var.clear();
		var.push_back(boost::lexical_cast<std::string>(this->captureIndex));
	}
	
	/**
	 * Returns a replacement function object from the given string and its parsed replacement pair.
	 * 
	 * @param[in] str - string representation of the replacement pair
	 * @param[in] replacement - replacement pair to use
	 * @return replacement function object
	 * @see pp::StringLiteral::functionReplace
	 */
	static StringLiteralFunctionPair getReplacementFunction(const std::string & str, const StringLiteralReplacementPair & replacement) {
		return StringLiteralFunctionPair(str, phx::bind(&pp::StringLiteral::functionReplace, phx::placeholders::_1, replacement));
	}
	
	/**
	 * Returns a substring function object from the given string and its parsed substring parameters.
	 * 
	 * @param[in] str - string representation of the substring parameters
	 * @param[in] start - start position of the substring
	 * @param[in] len - optional length of the substring
	 * @return substring function object
	 * @see pp::StringLiteral::functionSubstr
	 */
	static StringLiteralFunctionPair getSubstrFunction(const std::string & str, const int start, const boost::optional<int> len) {
		return StringLiteralFunctionPair(str, phx::bind(&pp::StringLiteral::functionSubstr, phx::placeholders::_1, start, len));
	}
};


} /* namespace parser */
} /* namespace pp */


#endif /* __PP_PARSER_STRINGLITERAL_HPP__ */
