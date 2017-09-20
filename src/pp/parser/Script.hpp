/**
 * @file Script.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-10-31
 * @version 2016-12-30
 */
#ifndef __PP_PARSER_SCRIPT_HPP__
#define __PP_PARSER_SCRIPT_HPP__


#include <set>
#include <sstream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/foreach.hpp>
#include <boost/locale.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>
#include <boost/spirit/repository/include/qi_seek.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/parser/spirit/Gap.hpp>
#include <pcf/parser/spirit/IfPragma.hpp>
#include <pcf/parser/spirit/Name.hpp>
#include <pcf/path/Utility.hpp>
#include "../Script.hpp"
#include "../Utility.hpp"
#include "StringLiteral.hpp"
#include "Utility.hpp"


namespace pp {
namespace parser {


namespace fusion = boost::fusion;
namespace phx = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace encoding = boost::spirit::standard;


/**
 * Skip grammar of the parallel processor script.
 *
 * @tparam Iterator - multi-pass forward iterator type
 */
template <typename Iterator>
struct ScriptSkipParser : qi::grammar<Iterator> {
	qi::rule<Iterator> skipper; /**< The skip grammar. */
	
	/**
	 * Default constructor.
	 */
	ScriptSkipParser() : ScriptSkipParser::base_type(skipper, "skipper") {
		using boost::spirit::repository::qi::seek;
		using qi::blank;
		using qi::char_;
		using qi::lit;
		using qi::eoi;
		using qi::eol;
		using pcf::parser::spirit::name;
		
		skipper = +(
			/* filling blanks */
			  blank
			/* end-of-line */
			| eol
			/* comment line */
			| (lit('#') > name("end-of-line")[seek[eol | eoi]])
			/* block comment */
			| (lit("/*") > name("*/")[seek[lit("*/")]])
		);
	}
};


/**
 * Grammar of the parallel processor script.
 *
 * @tparam Iterator - multi-pass forward iterator type
 * @tparam Skipper - Boost Spirit Qi skip grammar (e.g. whitespace)
 * @remarks http://stackoverflow.com/questions/3066701/boost-spirit-semantic-action-parameters
 * @see ScriptSkipParser
 */
template <typename Iterator, typename Skipper>
struct Script : qi::grammar<Iterator, Skipper> {
	pp::Script & script; /**< Script instance. */
	boost::filesystem::path sourcePath; /**< Source file location. */
	std::string shellName; /**< ID of the used execution shell. */
	pp::Shell shell; /**< Intermediate execution shell description. */
	StringLiteral<Iterator> stringLiteralGrammar; /**< Grammar of a string literal. */
	/* simple grammars */
	qi::rule<Iterator> bom; /**< Byte-order-mark. */
	qi::rule<Iterator> pragmaPrefix;
	qi::rule<Iterator> assign;
	qi::rule<Iterator> specifyGroup;
	qi::rule<Iterator> beginGroup;
	qi::rule<Iterator> endGroup;
	qi::rule<Iterator> temporaryDestination;
	qi::rule<Iterator, char()> beginArray;
	qi::rule<Iterator, char()> endArray;
	qi::rule<Iterator> endOfInput;
	qi::rule<Iterator> endOfLine;
	qi::rule<Iterator> beginBooleanGroup;
	qi::rule<Iterator> endBooleanGroup;
	qi::rule<Iterator> booleanNot;
	qi::rule<Iterator> booleanOr;
	qi::rule<Iterator> booleanAnd;
	qi::rule<Iterator> booleanIs;
	qi::rule<Iterator> booleanIsNot;
	qi::rule<Iterator> booleanIsLike;
	qi::rule<Iterator> booleanIsNotLike;
	qi::rule<Iterator> forceBuild;
	qi::rule<Iterator> invertFilter;
	qi::rule<Iterator> beginProcessGroup;
	qi::rule<Iterator> endProcessGroup;
	qi::rule<Iterator> addDependencyOutput;
	qi::rule<Iterator> processParallel;
	qi::rule<Iterator> processDependency;
	qi::rule<Iterator> singleQuote;
	qi::rule<Iterator> doubleQuote;
	qi::rule<Iterator> escapeCharacter;
	qi::rule<Iterator> parameterDelimiter;
	/* literals (same as variable name) */
	qi::rule<Iterator> l_asterisk;
	qi::rule<Iterator> l_enable;
	qi::rule<Iterator> l_disable;
	qi::rule<Iterator> l_verbosity;
	qi::rule<Iterator> l_shell;
	qi::rule<Iterator> l_include;
	qi::rule<Iterator> l_import;
	qi::rule<Iterator> l_and;
	qi::rule<Iterator> l_or;
	qi::rule<Iterator> l_is;
	qi::rule<Iterator> l_not;
	qi::rule<Iterator> l_set;
	qi::rule<Iterator> l_file;
	qi::rule<Iterator> l_directory;
	qi::rule<Iterator> l_regex;
	qi::rule<Iterator> l_like;
	qi::rule<Iterator> l_true;
	qi::rule<Iterator> l_false;
	qi::rule<Iterator> l_if;
	qi::rule<Iterator> l_else;
	qi::rule<Iterator> l_end;
	qi::rule<Iterator> l_unset;
	qi::rule<Iterator> l_process;
	qi::rule<Iterator> l_foreach;
	qi::rule<Iterator> l_all;
	qi::rule<Iterator> l_none;
	qi::rule<Iterator> l_execution;
	/* complex grammars */
	qi::rule<Iterator, std::string()> arrayIndex;
	qi::rule<Iterator, std::string()> idString;
	qi::rule<Iterator, std::string()> processId;
	qi::rule<Iterator, std::string()> executionId;
	qi::rule<Iterator, size_t()> uintValue;
	qi::rule<Iterator, pp::StringLiteralCaptureVector(bool, char)> stringLiteral;
	qi::rule<Iterator, pp::StringLiteral(bool, bool), qi::locals<Iterator> > valueString;
	qi::rule<Iterator, std::string(), qi::locals<char> > rawPatternString;
	qi::rule<Iterator, pp::StringLiteral(), qi::locals<Iterator> > patternString;
	qi::rule<Iterator, pp::StringLiteral(bool, bool), qi::locals<Iterator> > nonEmptyValueString;
	qi::rule<Iterator, pp::StringLiteral(), qi::locals<Iterator> > matchAnyRegexString;
	qi::rule<Iterator, pp::StringLiteral(bool &)> inputFileFilter;
	qi::rule<Iterator, void(), qi::locals<bool, Verbosity, Iterator, std::string>, Skipper> pragma;
	qi::rule<Iterator, bool(), qi::locals<boost::optional<pp::StringLiteral> /* _a */, bool /* _b */>, Skipper> beCheck;
	qi::rule<Iterator, bool(bool), Skipper> beAnd;
	qi::rule<Iterator, bool(), Skipper> beAndOrTerm;
	qi::rule<Iterator, bool(bool), Skipper> beOr;
	qi::rule<Iterator, bool(), Skipper> beTerm;
	qi::rule<Iterator, bool(), Skipper> beExpression;
	qi::rule<Iterator> pragmaEnd;
	qi::rule<Iterator, void(), qi::locals<Iterator>, Skipper> variableRemoval;
	qi::rule<Iterator, void(bool, bool), qi::locals<std::string, pp::StringLiteral>, Skipper> variableAssignment;
	qi::rule<Iterator, void(std::string, bool), qi::locals<std::string, pp::StringLiteral>, Skipper> patternAssignment;
	qi::rule<Iterator, void(pp::ProcessBlock &, bool), qi::locals<std::string, pp::StringLiteral, bool>, Skipper> destinationVariable;
	qi::rule<Iterator, void(pp::ProcessBlock &), qi::locals<std::string, pp::StringLiteral>, Skipper> dependencyVariable;
	qi::rule<Iterator, std::string()> commandLineStart;
	qi::rule<Iterator, std::string()> commandLineEnd;
	qi::rule<Iterator, pp::Command(), qi::locals<Iterator> > command;
	qi::rule<Iterator, pp::ProcessBlock(), qi::locals<ProcessBlock::Type, pp::StringLiteral, bool>, Skipper> processBlock;
	qi::rule<Iterator, pp::Process *(), Skipper> processPragmaInclude;
	qi::rule<Iterator, pp::Process(), Skipper> process;
	qi::rule<Iterator, void(pp::StringLiteralVector &, bool &), Skipper> processParameterList;
	qi::rule<Iterator, pp::ProcessNode(), qi::locals<std::string /* _a */, bool /* _b */>, Skipper> executionElement;
	qi::rule<
		Iterator,
		pp::ProcessElement(),
		qi::locals<std::string /* _a */, pp::StringLiteralVector /* _b */, bool /* _c */, Iterator /* _c */, VariableMap /* _d */, bool /* _e */>,
		Skipper
	> processElement;
	qi::rule<Iterator, pp::ProcessNode(), Skipper> pnLeaf;
	qi::rule<Iterator, pp::ProcessNode(pp::ProcessNode), Skipper> pnParallel;
	qi::rule<Iterator, pp::ProcessNode(pp::ProcessNode), Skipper> pnDependencies;
	qi::rule<Iterator, pp::ProcessNode(), Skipper> pnTerm;
	qi::rule<Iterator, pp::ProcessNode(), qi::locals<pp::ProcessNode>, Skipper> pnExpression;
	qi::rule<Iterator, pp::Execution *(), Skipper> executionPragmaInclude;
	qi::rule<Iterator, void(pp::Execution &), Skipper> executionPragmaIf;
	qi::rule<Iterator, void(pp::Execution &), Skipper> executionExpression;
	qi::rule<Iterator, pp::Execution(), Skipper> execution;
	qi::rule<Iterator, Skipper> globalPragmaIf;
	qi::rule<Iterator, Skipper> globalExpression;
	qi::rule<Iterator, Skipper> scriptGrammar; /**< Grammar of the complete script. */
	
	/**
	 * Constructor.
	 *
	 * @param[in,out] s - reference to the used script instance
	 * @param[in] p - path of the script file
	 */
	Script(pp::Script & s, const boost::filesystem::path & p) :
		Script::base_type(scriptGrammar, "parallel processor script"),
		script(s),
		sourcePath(p)
	{
		/* use these Spirit terminals and Phoenix actors */
		using boost::spirit::repository::qi::iter_pos;
		using boost::spirit::repository::qi::seek;
		using encoding::char_;
		using phx::at_c;
		using phx::construct;
		using phx::push_back;
		using phx::ref;
		using phx::val;
		using qi::eol;
		using qi::eoi;
		using qi::eps;
		using qi::alpha;
		using qi::alnum;
		using qi::as_string;
		using qi::blank;
		using qi::hold;
		using qi::uint_;
		using qi::lazy;
		using qi::lexeme;
		using qi::lit;
		using qi::omit;
		using qi::skip;
		using qi::string;
		using qi::unused;
		using qi::_1;
		using qi::_2;
		using qi::_3;
		using qi::_a;
		using qi::_b;
		using qi::_c;
		using qi::_d;
		using qi::_e;
		using qi::_f;
		using qi::_r1;
		using qi::_r2;
		using qi::_pass;
		using qi::_val;
		using pcf::parser::spirit::expectIfPragma;
		using pcf::parser::spirit::gap;
		using pcf::parser::spirit::name;
		
		bom.name("byte-order-mark");
		bom = -lit("\xEF\xBB\xBF"); /* byte-order-mark for UTF-8 */
		
		endOfInput.name("pragma, variable assignment, process or execution");
		endOfInput = eoi;
		
		endOfLine.name("end-of-line");
		endOfLine = eol;
		
		/* normal string literal omitting their match */
#define MAKE_LITERAL(var, x, y) \
		var.name(x); \
		var = lit(y);
		
		MAKE_LITERAL(pragmaPrefix, "@", '@');
		MAKE_LITERAL(assign, "=", '=');
		MAKE_LITERAL(specifyGroup, ":", ':');
		MAKE_LITERAL(beginGroup, "{", '{');
		MAKE_LITERAL(endGroup, "}", '}');
		MAKE_LITERAL(temporaryDestination, "~", '~');
		MAKE_LITERAL(beginBooleanGroup, "(", '(');
		MAKE_LITERAL(endBooleanGroup, ")", ')');
		MAKE_LITERAL(booleanNot, "!", '!');
		MAKE_LITERAL(booleanOr, "||", "||");
		MAKE_LITERAL(booleanAnd, "&&", "&&");
		MAKE_LITERAL(booleanIs, "=", '=');
		MAKE_LITERAL(booleanIsNot, "!=", "!=");
		MAKE_LITERAL(booleanIsLike, "~", '~');
		MAKE_LITERAL(booleanIsNotLike, "!~", "!~");
		MAKE_LITERAL(forceBuild, "!", '!');
		MAKE_LITERAL(invertFilter, "!", '!');
		MAKE_LITERAL(beginProcessGroup, "(", '(');
		MAKE_LITERAL(endProcessGroup, ")", ')');
		MAKE_LITERAL(addDependencyOutput, "+", '+');
		MAKE_LITERAL(processParallel, "|", '|');
		MAKE_LITERAL(processDependency, ">", '>');
		MAKE_LITERAL(singleQuote, "'", '\'');
		MAKE_LITERAL(doubleQuote, "\"", '"');
		MAKE_LITERAL(escapeCharacter, "\\", '\\');
		MAKE_LITERAL(parameterDelimiter, ",", ',');
		
#undef MAKE_LITERAL
		
		/* normal string literals that return their match */
#define MAKE_LITERAL(var, x, y) \
		var.name(x); \
		var = char_(y);
		
		MAKE_LITERAL(beginArray, "[", '[');
		MAKE_LITERAL(endArray, "]", ']');
		
#undef MAKE_LITERAL
		
		/* named string literal */
#define MAKE_LITERAL(x) \
		l_##x.name(#x); \
		l_##x = lit(#x)
		
		MAKE_LITERAL(enable);
		MAKE_LITERAL(disable);
		MAKE_LITERAL(verbosity);
		MAKE_LITERAL(shell);
		MAKE_LITERAL(include);
		MAKE_LITERAL(import);
		MAKE_LITERAL(is);
		MAKE_LITERAL(set);
		MAKE_LITERAL(file);
		MAKE_LITERAL(directory);
		MAKE_LITERAL(regex);
		MAKE_LITERAL(like);
		MAKE_LITERAL(end);
		MAKE_LITERAL(unset);
		MAKE_LITERAL(process);
		MAKE_LITERAL(foreach);
		MAKE_LITERAL(all);
		MAKE_LITERAL(none);
		MAKE_LITERAL(execution);
		
#undef MAKE_LITERAL

		l_asterisk.name("*");
		l_asterisk = lit('*');

		l_and.name("and");
		l_and = lit("and");
		
		l_or.name("or");
		l_or = lit("or");
		
		l_not.name("not");
		l_not = lit("not");
		
		l_true.name("true");
		l_true = lit("true");
		
		l_false.name("false");
		l_false = lit("false");
		
		l_if.name("if");
		l_if = lit("if");
		
		l_else.name("else");
		l_else = lit("else");
		
		arrayIndex.name("array index");
		arrayIndex = (char_("0") | (char_("1-9") >> *char_("0-9")));
		
		idString.name("identifier");
		idString = char_("_a-zA-Z") >> *char_("_a-zA-Z0-9");
		
		processId.name("valid process ID");
		processId = idString.alias();
		
		executionId.name("valid execution ID");
		executionId = idString.alias();
		
		uintValue.name("unsigned integer value");
		uintValue = (
			uint_[_val = _1] | idString[_pass = phx::bind<bool>(&Script::getVariableAsNum<size_t>, this, _val, _1)]
		);
		
		/* @param[in] _r1 - enable captures
		 * @param[in] _r2 - quoting character */
		stringLiteral.name("string literal");
		stringLiteral = (
			stringLiteralGrammar(
				phx::if_else( /* ternary: _r1 ? ENABLE_CAPTURES : STANDARD */
					_r1,
					val(pp::StringLiteral::ENABLE_CAPTURES),
					val(pp::StringLiteral::STANDARD)
				),
				construct<boost::optional<char> >(_r2),
				ref(this->script.config.fullRecursiveMatch)
			)
		);
		
		/* @param[in] _r1 - enable checking
		 * @param[in] _r2 - enable captures */
		valueString.name("string");
		valueString = (
			&(singleQuote | doubleQuote) >> (
				  (singleQuote >> omit[iter_pos[_a = _1]] >> -stringLiteral(_r2, '\'') > singleQuote)
				| (doubleQuote >> omit[iter_pos[_a = _1]] >> -stringLiteral(_r2, '"') > doubleQuote)
			)[_pass = phx::bind<bool>(&Script::getOptionalStringLiteral, this, _val, _1, _a, _r1)]
		);
		
		rawPatternString.name("replacement pattern");
		rawPatternString %= (
			   (&char_[_a = _1]) >> char_(_a) >> (*(!char_(_a) >> char_)) >> char_(_a)
			>> (*(!char_(_a) >> char_)) >> char_(_a)
		);
		
		patternString.name("replacement pattern");
		patternString = (
			omit[iter_pos[_a = _1]] >> rawPatternString[_pass = phx::bind<bool>(&Script::getRawStringLiteral, this, _val, _1, _a)]
		);
		
		/* @param[in] _r1 - enable checking
		 * @param[in] _r2 - enable captures */
		nonEmptyValueString.name("non-empty string");
		nonEmptyValueString = (
			&(singleQuote | doubleQuote) >> (
				  (singleQuote >> omit[iter_pos[_a = _1]] > stringLiteral(_r2, '\'') > singleQuote)
				| (doubleQuote >> omit[iter_pos[_a = _1]] > stringLiteral(_r2, '"') > doubleQuote)
			)[_pass = phx::bind<bool>(&Script::getStringLiteral, this, _val, _1, _a, _r1)]
		);
		
		matchAnyRegexString.name("nothing");
		matchAnyRegexString = (
			omit[iter_pos[_a = _1]] >> eps[_pass = phx::bind<bool>(&Script::getRawStringLiteral, this, _val, std::string(".*"), _a)]
		);
		
		variableRemoval.name("variable removal");
		variableRemoval = (
			l_unset >> omit[iter_pos[_a = _1]] > idString[phx::bind(&Script::unsetVariable, this, _1, _a)]
			>> *(parameterDelimiter >> omit[iter_pos[_a = _1]] > idString[phx::bind(&Script::unsetVariable, this, _1, _a)])
		);
		
		/* @param[in] _r1 - enable checking
		 * @param[in] _r2 - enable captures */
		variableAssignment.name("variable assignment");
		variableAssignment = (
			(idString[_a = _1] >> assign > valueString(_r1, _r2)[_b = _1])[
				_pass = phx::bind<bool>(&Script::setVariable, this, _a, _b, _r1, false)
			]
		);
		
		/* @param[in] _r1 - variable name
		 * @param[in] _r2 - true to enable warning on variable value overwrite */
		patternAssignment.name("replacement pattern assignment");
		patternAssignment = (
			(
				as_string[
					string(_r1) >> -(beginArray > arrayIndex > endArray)
				][_a = _1] > assign > patternString[_b = _1]
			)[_pass = phx::bind<bool>(&Script::setVariable, this, _a, _b, false, _r2)]
		);
		
		pragma.name("pragma");
		pragma = lexeme[
			pragmaPrefix
			>> (
				/* enable/disable */
				(
					((l_enable[_a = val(true)] | l_disable[_a = val(false)]) > gap) > (
						  lit("environment-variables")[phx::bind(&Script::switchEnvironmentVariables, this, _a)]
						| lit("variable-checking")    [ref(this->script.config.variableChecking)    = _a]
						| lit("command-checking")     [ref(this->script.config.commandChecking)     = _a]
						| lit("nested-variables")     [ref(this->script.config.nestedVariables)     = _a]
						| lit("full-recursive-match") [ref(this->script.config.fullRecursiveMatch)  = _a]
						| lit("remove-temporaries")   [ref(this->script.config.removeTemporaries)   = _a]
						| lit("clean-up-incompletes") [ref(this->script.config.cleanUpIncompletes)  = _a]
						| lit("remove-remains")       [ref(this->script.config.removeRemains)       = _a]
					)
				)
				/* verbosity */
				| ((l_verbosity > gap) > verbosityValue[phx::bind(&Script::setVerbosity, this, _1)])
				/* shell */
				| ((l_shell > gap) >> iter_pos[_c = _1] >> (
					skip[
						specifyGroup[phx::bind(&Script::defShellReset, this)]
						> idString[phx::bind(&Script::defShellSetName, this, _1)]
						> beginGroup[phx::bind(&Script::enterScope, this)]
						> name("variable assignment")[(*((!endGroup) >> (
							patternAssignment(std::string("replace"), true) | variableAssignment(false, false)
						)) >> iter_pos[_c = _1])[_pass = phx::bind<bool>(&Script::defShellParameters, this, _c)]]
						> endGroup[phx::bind(&Script::leaveScope, this)]
					]
					| idString[_pass = phx::bind<bool>(&Script::useShell, this, _1, _c)]
				))
				/* error|warn|info|debug */
				| (((verbosityCommand[_b = _1] > gap) >> iter_pos[_c = _1]) > name("identifier | string literal | *")[
					  idString[phx::bind(&Script::printVariable, this, _b, _c, _1)]
					| valueString(true, false)[phx::bind(&Script::printStringLiteral, this, _b, _c, _1)]
					| l_asterisk[phx::bind(&Script::printAllVariables, this, _b, _c)]
				])
				/* import */
				| ((l_import > gap) > nonEmptyValueString(true, false)[_pass = phx::bind<bool>(&Script::readImport, this, _1)])
				/* include */
				| ((l_include > gap) > nonEmptyValueString(true, false)[_pass = phx::bind<bool>(&Script::readInclude, this, _1)])
			)
		];
		
		beCheck.name("boolean check");
		beCheck = (
			(
				  idString[_val = phx::bind<bool>(&Script::beGetVariable, this, _a, _1)]
				| (valueString(true, false)[_a = construct<boost::optional<pp::StringLiteral> >(_1)] >> eps[_val = val(true)])
			)
			>> -(eps[_b = val(false)] >> (
				(
					(((l_is >> l_not) | booleanIsNot)[_b = val(true)] | (l_is | booleanIs))
					>> !l_like >> (
						  l_set                   [_val = _val ^ _b]
						| valueString(true, false)[_val = phx::bind<bool>(&Script::beMatchLiteral, _a, _1) ^ _b]
						| l_file                  [_val = phx::bind<bool>(&Script::beIsFile,       _a) ^ _b]
						| l_directory             [_val = phx::bind<bool>(&Script::beIsDirectory,  _a) ^ _b]
						| l_regex                 [_val = phx::bind<bool>(&Script::beIsRegex,      _a) ^ _b]
						| l_true                  [_val = phx::bind<bool>(&Script::beIsTrue,       _a) ^ _b]
						| l_false                 [_val = phx::bind<bool>(&Script::beIsFalse,      _a) ^ _b]
						/* check last to prevent conflict with variable named like a keyword */
						| idString                [_val = phx::bind<bool>(&Script::beMatchVariable, this, _a, _1) ^ _b]
					)
				) | (
					(((l_is >> l_not >> l_like) | booleanIsNotLike)[_b = val(true)] | ((l_is >> l_like) | booleanIsLike))
					> valueString(true, false)[_val = phx::bind<bool>(&Script::beMatchRegex, _a, _1) ^ _b]
				)
			))
		);
	
		beAnd.name("\"A and B\" \"A && B\"");
		beAnd = (l_and | booleanAnd) > beTerm[_val = _r1 && _1];
		
		beAndOrTerm.name("boolean and or term");
		beAndOrTerm = beTerm[_val = _1] >> *(beAnd(_val)[_val = _1]);
		
		/* using beExpression instead of beAndOrTerm leads into an endless loop here */
		beOr.name("\"A or B\" \"A || B\"");
		beOr = (l_or | booleanOr) > beAndOrTerm[_val = _r1 || _1];
		
		beTerm.name("boolean expression term");
		beTerm = (
			   eps[_val = false] >> -((l_not | booleanNot)[_val = true])
			>> ((beginBooleanGroup > beExpression > endBooleanGroup) | (eps > beCheck))[_val = _val ^ _1]
		);
		
		beExpression.name("boolean expression");
		beExpression = beAndOrTerm[_val = _1] >> *(beOr(_val)[_val = _1]);
		
		pragmaEnd.name("@end");
		pragmaEnd = pragmaPrefix >> l_end;
		
		/* @param[out] _r1 - output variable for invert filter flag */
		inputFileFilter.name("input file filter");
		inputFileFilter = eps[_r1 = false] >> (
			 (invertFilter[_r1 = true] > nonEmptyValueString(true, false))
			| nonEmptyValueString(true, false)
			| matchAnyRegexString
		)[_val = _1];
		
		/* @param[out] _r1 - output variable for process block
		 * @param[in] _r2 - enable syntactic temporaries */
		destinationVariable.name("destination variable assignment");
		destinationVariable = (
			omit[
				/* default: destination file remains */
				/* ~: destination file is deleted at the end on success */
				  (eps(_r2) >> temporaryDestination[_c = val(true)] | eps[_c = val(false)])
				/* disable temporaries if option for this is disabled */
				>> eps[_c = _c && phx::ref(this->script.config.removeTemporaries)]
				>> as_string[string("destination") >> -(beginArray > arrayIndex > endArray)][_a = _1]
				>  assign /* not sure why this can not be a normal sequence operator */
				>> valueString(false, true)[_b = _1]
			][_pass = phx::bind<bool>(&Script::setDestinationVariable, this, _r1, _a, _b, _c)]
		);
		
		/* @param[out] _r1 - output variable for process block */
		dependencyVariable.name("dependency variable assignment");
		dependencyVariable = (
			omit[
				   as_string[string("dependency") >> -(beginArray > arrayIndex > endArray)][_a = _1]
				>  assign
				>> valueString(false, true)[_b = _1]
			][_pass = phx::bind<bool>(&Script::setDependencyVariable, this, _r1, _a, _b)]
		);
		
		commandLineStart.name("command");
		commandLineStart = (
			+(!(*blank >> ((-(escapeCharacter >> *blank)) >> endOfLine)) >> char_)
		);
		
		commandLineEnd.name("end-of-command-line");
		commandLineEnd = (
			*blank >> escapeCharacter >> *blank >> endOfLine >> qi::attr('\n')
		);
		
		command.name("command");
		command = (
			   iter_pos[_a = _1]
			>> as_string[
				commandLineStart >> *(commandLineEnd > commandLineStart)
			][_pass = phx::bind<bool>(&Script::setCommand, this, _val, _1, _a)]
		);
		
		processBlock.name("foreach \"regex\" { ... } | all \"regex\" { ... } | none { ... }");
		processBlock = (
			(iter_pos[phx::bind(&Script::setLineInfo<ProcessBlock>, _val, _1)]
			>> eps[phx::bind(&Script::setLocalVariables, this, _val)]
			>> (l_foreach[_a = val(ProcessBlock::FOREACH)] | l_all[_a = val(ProcessBlock::ALL)] | l_none[_a = val(ProcessBlock::NONE)]))
			>> (
				  (eps(_a == val(ProcessBlock::NONE))[phx::bind(&pp::ProcessBlock::type, _val) = _a])
				/* regular expression as input file filter */
				| (eps > inputFileFilter(_c)[phx::bind(&Script::setProcessBlockInfo, _val, _a, _1, _c)])
			)
			> beginGroup[(phx::bind(&Script::switchScope, this), phx::bind(&Script::enterScope, this))]
			> *((!endGroup) >> (
				  destinationVariable(_val, _a != val(ProcessBlock::NONE))
				| dependencyVariable(_val)
				| (eps(ref(this->script.config.nestedVariables)) >> variableAssignment(false, false))
				| command[phx::bind(&pp::ProcessBlock::addCommand, _val, _1)]
			))
			> endGroup[(phx::bind(&Script::leaveScope, this), phx::bind(&Script::switchScope, this))]
		);
		
		processPragmaInclude.name("@include <process-id>");
		processPragmaInclude = (
			lexeme[pragmaPrefix > l_include > gap > processId[_pass = phx::bind<bool>(&Script::getProcess, this, _val, _1)]]
		);
		
		process.name("process : <id> { ... }");
		process = (
			iter_pos[phx::bind(&Script::setLineInfo<Process>, _val, _1)]
			>> l_process >> specifyGroup > processId[phx::bind(&Script::setProcessId, _val, _1)]
			> beginGroup[phx::bind(&Script::enterScope, this)]
			> *(
				  processBlock[phx::bind(&Script::addProcessBlock, _val, _1)]
				| processPragmaInclude[phx::bind(&Script::includeProcessBlocks, _val, _1)]
			)
			> endGroup[phx::bind(&Script::leaveScope, this)]
		);
		
		processParameterList.name("process parameters");
		processParameterList = (
			omit[-(addDependencyOutput[_r2 = val(true)])]
			>> (
				(
					(((eps(_r2) >> parameterDelimiter) | eps(!_r2))
					> name("process parameter")[
						  variableAssignment(false, false)
						| nonEmptyValueString(true, false)[push_back(_r1, _1)]
						| idString[_pass = phx::bind<bool>(&Script::addVariableToCurrentScope, this, _1, true)]
					]) >> *(parameterDelimiter > name("process parameter")[
						  variableAssignment(false, false)
						| nonEmptyValueString(true, false)[push_back(_r1, _1)]
						| idString[_pass = phx::bind<bool>(&Script::addVariableToCurrentScope, this, _1, true)]
					])
				) | (
					eps > name(")' or ', <process parameter>")[&endProcessGroup]
				)
			)
		);
		
		executionElement.name("execution element");
		executionElement = (
			omit[
				   -(forceBuild[_b = val(true)])
				>> executionId[_a = _1]
			][_pass = phx::bind<bool>(&Script::setExecutionElement, this, _val, _a, _b)]
		);
		
		processElement.name("process element");
		processElement = (
			omit[
				   eps[_c = val(false)]
				>> iter_pos[_d = _1, _f = val(false)]
				>> -(forceBuild[_f = val(true)])
				>> processId[_a = _1]
				>> -(
					  beginProcessGroup[phx::bind(&Script::enterScope, this)]
					> processParameterList(_b, _c)
					> endProcessGroup[phx::bind(&Script::getCurrentScope, this, _e)]
					>> eps[phx::bind(&Script::leaveScope, this)]
				)
			][_pass = phx::bind<bool>(&Script::setProcessElement, this, _val, _a, _b, _c, _d, _e, _f)]
		);
		
		pnLeaf.name("process chain element");
		pnLeaf = (
			  executionElement[phx::bind(&Script::setProcessParallel, _val, _1)]
			| processElement[phx::bind(&Script::setLeafProcess, _val, _1)]
		);
		
		pnParallel.name("parallel process group");
		pnParallel = (
			eps[phx::bind(&Script::addProcessNode, _val, _r1)]
			>> +(
				processParallel > pnTerm[phx::bind(&Script::addProcessNode, _val, _1)]
			)
		);
		
		pnDependencies.name("process dependency chain");
		pnDependencies = (
			eps[_val = _r1]
			>> +(
				processDependency > pnTerm[_val = phx::bind<ProcessNode>(&Script::getDepProcessNode, _val, _1)]
			)
		);
		
		pnTerm.name("process or process group");
		pnTerm = (beginProcessGroup > pnExpression > endProcessGroup) | pnLeaf;
		
		pnExpression.name("process chain");
		pnExpression = (
			 pnTerm[_a = _1] >> (
				  pnDependencies(_a)[_val = _1]
				| pnParallel(_a)[phx::bind(&Script::setProcessParallel, _val, _1)]
				| eps[_val = _a]
			)
		);
		
		executionPragmaInclude.name("@include <execution-id>");
		executionPragmaInclude = (
			lexeme[pragmaPrefix >> l_include > gap > executionId[_pass = phx::bind<bool>(&Script::getExecution, this, _val, _1)]]
		);
		
		executionPragmaIf.name("pragma if statement");
		executionPragmaIf = expectIfPragma(
			lexeme[pragmaPrefix >> l_if],                /* if */
			beExpression.alias(),                        /* expression */
			unused,                                      /* then */
			lexeme[pragmaPrefix >> l_else] >> l_if,      /* else if */
			lexeme[pragmaPrefix >> l_else],              /* else */
			omit[char_],                                 /* block-skipper (handles skip grammar too) */
			pragmaEnd.alias()                            /* end */
		)[*(executionExpression(_r1))];                  /* block */
		
		executionExpression.name("pragma or process chain");
		executionExpression = (
			  (pnExpression | pnLeaf)[phx::bind(&Script::addExecutionProcessNode, _r1, _1)]
			| executionPragmaInclude[phx::bind(&Script::includeExecutionNodes, _r1, _1)]
			| executionPragmaIf(_r1)
		);
		
		execution.name("execution : <target> { ... }");
		execution = (
			iter_pos[phx::bind(&Script::setLineInfo<Execution>, _val, _1)]
			>> l_execution >> specifyGroup > executionId[phx::bind(&Script::setExecutionId, _val, _1)]
			> beginGroup[phx::bind(&Script::enterScope, this)]
			> *executionExpression(_val)
			> endGroup[phx::bind(&Script::leaveScope, this)]
		);
		
		globalPragmaIf.name("pragma if statement");
		globalPragmaIf = expectIfPragma(
			lexeme[pragmaPrefix >> l_if],                /* if */
			beExpression.alias(),                        /* expression */
			unused,                                      /* then */
			lexeme[pragmaPrefix >> l_else] >> l_if,      /* else if */
			lexeme[pragmaPrefix >> l_else],              /* else */
			omit[char_],                                 /* block-skipper (handles skip grammar too) */
			pragmaEnd.alias()                            /* end */
		)[*globalExpression];                            /* block */
		
		globalExpression.name("pragma, variable assignment, variable removal, process or execution");
		globalExpression = (
			  pragma
			| globalPragmaIf
			| variableAssignment(true, false)
			| variableRemoval
			| process[phx::bind(&Script::addProcess, this, _1)]
			| execution[phx::bind(&Script::addExecution, this, _1)]
		);
		
		scriptGrammar = eps > bom
			> *globalExpression
			> endOfInput;
		
/*#define DEBUG_NODE(x) \
		qi::on_success(x, ref(std::cout) << "OK(" << #x << ")" << std::endl); \
		qi::on_error<qi::fail>(x, ref(std::cout) << "NOK(" << #x << "): " << phx::construct<std::string>(_1, _3) << std::endl)
		DEBUG_NODE(scriptGrammar);*/
	}

private:
	/**
	 * Sets the script location to a given object from the passed iterator position.
	 *
	 * @param[in,out] var - set script location for this object
	 * @param[in] it - extract script location from this iterator
	 * @tparam T - type of the object to set the script location for (needs to support setLineInfo(pp::LineInfo))
	 */
	template <typename T>
	static void setLineInfo(T & var, const Iterator & it) {
		const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
		const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
		var.setLineInfo(scriptPos);
	}
	
	/**
	 * Returns a string literal from the given optional raw string literal data returned by the
	 * string literal grammar.
	 *
	 * @param[out] output - set this referenced variable
	 * @param[in] slcv - raw string literal data
	 * @param[in] it - extract script location from this iterator
	 * @param[in] enableChecking - set to enable checking of referenced variable replacement
	 * @return true if the output variable was set with a valid value, else false
	 */
	bool getOptionalStringLiteral(pp::StringLiteral & output, const boost::optional<pp::StringLiteralCaptureVector> & slcv, const Iterator & it, const bool enableChecking = true) {
		const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
		const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
		if ( ! slcv ) {
			output = pp::StringLiteral(pp::StringLiteralCaptureVector(), scriptPos, false);
			return false;
		}
		std::string unknownVariable;
		output = pp::StringLiteral(*slcv, scriptPos);
		const bool invalidAccess = ( ! output.replaceVariables(unknownVariable, this->script.vars) );
		if ( invalidAccess ) {
			if ( enableChecking ) {
				if ( this->script.config.variableChecking ) {
					BOOST_THROW_EXCEPTION(
						pcf::exception::SymbolUnknown()
						<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(output.getLineInfo()) + ": Error: Trying to access unknown variable \"" + unknownVariable + "\".")
					);
					return false;
				} else {
					std::cerr << output.getLineInfo() << ": Warning: Trying to access unknown variable \"" + unknownVariable + "\"." << std::endl;
					output.fold(true, this->script.vars.getDynamicVariables()); /* removes unresolved variable references */
				}
			}
		}
		if ( ! output.isSet() ) {
			std::cerr << scriptPos << ": Error: Failed to parse string literal: " << output.getVarString() << std::endl;
			return false;
		}
		return true;
	}
	
	/**
	 * Returns a string literal from the given raw string literal data returned by the
	 * string literal grammar.
	 *
	 * @param[out] output - set this referenced variable
	 * @param[in] slcv - raw string literal data
	 * @param[in] it - extract script location from this iterator
	 * @param[in] enableChecking - set to enable checking of referenced variable replacement
	 * @return true if the output variable was set with a valid value, else false
	 */
	bool getStringLiteral(pp::StringLiteral & output, const pp::StringLiteralCaptureVector & slcv, const Iterator & it, const bool enableChecking = true) {
		return this->getOptionalStringLiteral(output, boost::optional<pp::StringLiteralCaptureVector>(slcv), it, enableChecking);
	}
	
	/**
	 * Returns a string literal by parsing the given raw string.
	 *
	 * @param[out] output - set this referenced variable
	 * @param[in] str - parse this string as raw string literal
	 * @param[in] it - extract script location from this iterator
	 * @return true if the output variable was set, else false
	 */
	bool getRawStringLiteral(pp::StringLiteral & output, const std::string & str, const Iterator & it) {
		const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
		const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
		output.setRawString(str, scriptPos);
		if ( ! output.isSet() ) {
			std::cerr << scriptPos << ": Failed to parse string literal: " << str << std::endl;
		}
		return output.isSet();
	}
	
	/**
	 * Returns the string literal value of a given variable name.
	 * 
	 * @param[out] output - set this referenced variable
	 * @param[in] str - name of the requested variable
	 * @return true if the output variable was set, else false
	 */
	bool beGetVariable(boost::optional<pp::StringLiteral> & output, const std::string & str) {
		boost::optional<pp::StringLiteral &> value(this->script.vars.get(str));
		if ( value ) {
			output.reset(*value);
		} else {
			output.reset();
		}
		return (output && output->isSet());
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and an
	 * existing file path.
	 *
	 * @param[in] value - value to test
	 * @return true if the value is set and an existing file, else false
	 */
	static bool beIsFile(const boost::optional<pp::StringLiteral> & value) {
		boost::system::error_code ec;
		bool output;
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		output = boost::filesystem::is_regular_file(boost::filesystem::path(value->getString(), pcf::path::utf8), ec);
		if ( ec ) return false;
		return output;
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and an
	 * existing directory path.
	 *
	 * @param[in] value - value to test
	 * @return true if the value is set and an existing directors, else false
	 */
	static bool beIsDirectory(const boost::optional<pp::StringLiteral> & value) {
		boost::system::error_code ec;
		bool output;
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		output = boost::filesystem::is_directory(boost::filesystem::path(value->getString(), pcf::path::utf8), ec);
		if ( ec ) return false;
		return output;
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and a
	 * valid regular expression.
	 *
	 * @param[in] value - value to test
	 * @return true if the value is set and a valid regular expression, else false
	 */
	static bool beIsRegex(const boost::optional<pp::StringLiteral> & value) {
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		try {
			const std::wstring wstr(boost::locale::conv::utf_to_utf<wchar_t>(value->getString()));
			boost::wregex regex(wstr, boost::regex_constants::perl | boost::regex_constants::no_except);
			if (regex.status() != 0) return false;
		} catch (...) {
			return false;
		}
		return true;
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and a
	 * "true" string. Also returns true if the value is "1".
	 *
	 * @param[in] value - value to test
	 * @return true if the value is set and a "true" string, else false
	 */
	static bool beIsTrue(const boost::optional<pp::StringLiteral> & value) {
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		const std::string str(value->getString());
		if ( str == "1" ) return true;
		return boost::algorithm::iequals(str, "true");
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and a
	 * "false" string. Also returns true if the value is "0".
	 *
	 * @param[in] value - value to test
	 * @return true if the value is set and a "false" string, else false
	 */
	static bool beIsFalse(const boost::optional<pp::StringLiteral> & value) {
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		const std::string str(value->getString());
		if ( str == "0" ) return true;
		return boost::algorithm::iequals(str, "false");
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and the
	 * same as a given variable.
	 *
	 * @param[in] value - value to test
	 * @param[in] varName - check against this variable
	 * @return true if the value is set and the same as a given variable, else false
	 */
	bool beMatchVariable(const boost::optional<pp::StringLiteral> & value, const std::string & varName) {
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		boost::optional<pp::StringLiteral> var;
		if ( ! beGetVariable(var, varName) ) return false;
		return *value == *var;
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and the
	 * same as a given string literal.
	 *
	 * @param[in] value - value to test
	 * @param[in] varName - check against this string literal
	 * @return true if the value is set and the same as a given string literal, else false
	 */
	static bool beMatchLiteral(const boost::optional<pp::StringLiteral> & value, const pp::StringLiteral & str) {
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		return *value == str;
	}
	
	/**
	 * Boolean expression evaluation function. Tests if the given string variable is set and matches
	 * the given regular expression.
	 *
	 * @param[in] value - value to test
	 * @param[in] regexStr - check against this regular expression
	 * @return true if the value is set and matches the given regular expression, else false
	 */
	static bool beMatchRegex(const boost::optional<pp::StringLiteral> & value, const pp::StringLiteral & regexStr) {
		if ( ! value ) return false;
		if ( ! value->isSet() ) return false;
		const std::wstring a(boost::locale::conv::utf_to_utf<wchar_t>(value->getString()));
		const std::wstring b(boost::locale::conv::utf_to_utf<wchar_t>(regexStr.getString()));
		try {
			boost::wregex regex(b, boost::regex_constants::perl);
			boost::wsmatch what;
			if ( boost::regex_match(a, what, regex) ) return true;
		} catch (const boost::regex_error & e) {
			throwRegexException(regexStr, e);
			return false;
		}
		return false;
	}
	
	/**
	 * Sets the default verbosity level.
	 *
	 * @param[in] verbosity - set this value
	 */
	void setVerbosity(const pp::Verbosity verbosity) {
		if ( ! this->script.config.lockedVerbosity ) {
			this->script.config.verbosity = verbosity;
		}
	}
	
	/**
	 * Resets the current shell ID and description for internal parsing.
	 */
	void defShellReset() {
		this->shellName = std::string();
		this->shell = pp::Shell();
	}
	
	/**
	 * Sets the current shell ID for internal parsing.
	 *
	 * @param[in] str - set shell ID to this value
	 */
	void defShellSetName(const std::string & str) {
		this->shellName = str;
	}
	
	/**
	 * Sets the current shell description parameters for internal parsing.
	 * The parameters are stored in the current variable scope.
	 *
	 * @param[in] it - extract script location from this iterator
	 * @return true on success, else false
	 */
	bool defShellParameters(const Iterator & it) {
		const static std::map<std::string, pp::Shell::Encoding> encodings = boost::assign::map_list_of
			("utf8",   pp::Shell::UTF8)
			("utf-8",  pp::Shell::UTF8)
			("utf16",  pp::Shell::UTF16)
			("utf-16", pp::Shell::UTF16)
		;
		const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
		const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
		boost::optional<pp::StringLiteral &> value;
		/* path */
		value = this->script.vars.get("path");
		if ( ! value || ! value->isSet() ) {
			std::cerr << scriptPos << ": Missing shell parameter \"path\"." << std::endl;
			return false;
		} else if ( value->isVariable() ) {
			std::cerr << value->getLineInfo() << ": Variable \"path\" is incomplete. Please check if all used variables are defined." << std::endl;
			return false;
		}
		this->shell.path = boost::filesystem::path(value->getString(), pcf::path::utf8);
		/* commandLine */
		value = this->script.vars.get("commandLine");
		if ( ! value || ! value->isSet() ) {
			std::cerr << scriptPos << ": Missing shell parameter \"commandLine\"." << std::endl;
			return false;
		}
		this->shell.cmdLine = *value;
		/* replace */
		{
			boost::optional<VariableMap &> currentScope(this->script.vars.getCurrentScope());
			if ( currentScope ) {
				VariableMap::const_iterator i = currentScope->lower_bound("replace");
				const VariableMap::const_iterator endI = currentScope->end();
				typedef std::map<ssize_t, std::pair<std::string, pp::StringLiteral> > ReplacementVarMap;
				ReplacementVarMap replacementVars;
				boost::optional<ssize_t> varIndex;
				/* get replacement variables sorted by index */
				for ( ; i != endI; ++i) {
					varIndex = parseVariableIndex(i->first, "replace");
					if ( varIndex ) {
						if ( i->second.isSet() ) {
							replacementVars[*varIndex] = std::pair<std::string, pp::StringLiteral>(i->first, i->second);
						}
					} else {
						break;
					}
				}
				/* set replacements ordered by index */
				BOOST_FOREACH(const ReplacementVarMap::value_type & repl, replacementVars) {
					try {
						if ( ! this->shell.addReplacement(repl.second.second.getString()) ) {
							std::cerr << repl.second.second.getLineInfo() << ": Wrong pattern format in \"" << repl.second.first << "\"." << std::endl;
							return false;
						}
					} catch (const pcf::exception::SyntaxError & e) {
						if (const std::string * errMsg = boost::get_error_info<pcf::exception::tag::Message>(e)) {
							std::cerr << repl.second.second.getLineInfo() << ": Wrong pattern format in \"" << repl.second.first << "\".\n" << *errMsg << std::endl;
						} else {
							std::cerr << repl.second.second.getLineInfo() << ": Wrong pattern format in \"" << repl.second.first << "\"." << std::endl;
						}
						return false;
					}
				}
			}
		}
		/* outputEncoding */
		value = this->script.vars.get("outputEncoding");
		if ( value && value->isSet() ) {
			if ( value->isVariable() ) {
				std::cerr << value->getLineInfo() << ": Variable \"outputEncoding\" is incomplete." << std::endl;
				return false;
			}
			std::map<std::string, pp::Shell::Encoding>::const_iterator encoding = encodings.find(boost::locale::to_lower(value->getString()));
			if (encoding == encodings.end()) {
				std::cerr << value->getLineInfo() << ": Given value for \"outputEncoding\" is invalid." << std::endl;
				return false;
			}
			this->shell.outputEncoding = encoding->second;
		} else {
			this->shell.outputEncoding = Shell::UTF8;
		}
#if defined(PCF_IS_WIN)
		/* raw */
		value = this->script.vars.get("raw");
		if ( value && value->isSet() ) {
			if ( value->isVariable() ) {
				std::cerr << value->getLineInfo() << ": Variable \"raw\" is incomplete." << std::endl;
				return false;
			}
			try {
				this->shell.raw = pcf::string::toBool(value->getString());
			} catch (const pcf::exception::InvalidValue &) {
				std::cerr << value->getLineInfo() << ": Given value for \"raw\" is not a boolean." << std::endl;
				return false;
			}
		} else {
			this->shell.raw = false;
		}
#else /* not Windows */
		this->shell.raw = false;
#endif /* Windows */
		/* set new shell (overwrite old one if already set) */
		this->script.shells[this->shellName] = boost::make_shared<Shell>(this->shell);
		return true;
	}
	
	/**
	 * Sets the current shell to the shell with the given ID for internal parsing.
	 *
	 * @param[in] str - use this shell ID
	 * @param[in] it - extract script location from this iterator
	 * @return true on success, else false
	 */
	bool useShell(const std::string & str, const Iterator & it) {
		if (this->script.shells.find(str) == this->script.shells.end()) {
			const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
			const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
			std::cerr << scriptPos << ": Given shell was not defined: " << str << std::endl;
			return false;
		}
		this->script.config.shell = str;
		return true;
	}
	
	/**
	 * Prints the given variable on the console if the verbosity is at most equal to the currently
	 * set verbosity level. Setting a value closer to ERROR will make the output more likely to pass
	 * the verbosity constraint.
	 *
	 * @param[in] verbosity - output at the verbosity level
	 * @param[in] it - extract script location from this iterator
	 * @param[in] var - output content of this variable
	 */
	void printVariable(const Verbosity verbosity, const Iterator & it, const std::string & var) {
		if (verbosity <= this->script.config.verbosity) {
			const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
			const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
			const boost::optional<pp::StringLiteral &> varVal = this->script.vars.get(var);
			std::string verbosityStr = boost::locale::to_lower(boost::lexical_cast<std::string>(verbosity));
			if ( ! verbosityStr.empty() ) verbosityStr[0] = static_cast<char>(::toupper(verbosityStr[0]));
			if ( ! varVal ) {
				if ( this->script.config.variableChecking ) {
					BOOST_THROW_EXCEPTION(
						pcf::exception::SymbolUnknown()
						<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(scriptPos) + ": Error: Trying to access unknown variable \"" + var + "\".")
					);
				} else {
					std::cerr << scriptPos << ": Warning: Trying to access unknown variable \"" + var + "\"." << std::endl;
				}
				return;
			}
			switch (verbosity) {
			case VERBOSITY_INFO:
				std::cout << scriptPos << ": " << verbosityStr << ": " << varVal->getVarString() << std::endl;
				break;
			case VERBOSITY_ERROR:
				BOOST_THROW_EXCEPTION(
					pcf::exception::Script()
					<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(scriptPos) + ": Error: " + varVal->getVarString())
				);
				break;
			default:
				std::cerr << scriptPos << ": " << verbosityStr << ": " << varVal->getVarString() << std::endl;
				break;
			}
		}
	}
	
	/**
	 * Prints the given string literal on the console if the verbosity is at most equal to the
	 * currently set verbosity level. Setting a value closer to ERROR will make the output more
	 * likely to pass the verbosity constraint.
	 *
	 * @param[in] verbosity - output at the verbosity level
	 * @param[in] it - extract script location from this iterator
	 * @param[in] literal - output this string literal
	 */
	void printStringLiteral(const Verbosity verbosity, const Iterator & it, const pp::StringLiteral & literal) {
		if (verbosity <= this->script.config.verbosity) {
			const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
			const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
			std::string verbosityStr = boost::locale::to_lower(boost::lexical_cast<std::string>(verbosity));
			if ( ! verbosityStr.empty() ) verbosityStr[0] = static_cast<char>(::toupper(verbosityStr[0]));
			switch (verbosity) {
			case VERBOSITY_INFO:
				std::cout << scriptPos << ": " << verbosityStr << ": " << literal.getVarString() << std::endl;
				break;
			case VERBOSITY_ERROR:
				BOOST_THROW_EXCEPTION(
					pcf::exception::Script()
					<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(scriptPos) + ": Error: " + literal.getVarString())
				);
				break;
			default:
				std::cerr << scriptPos << ": " << verbosityStr << ": " << literal.getVarString() << std::endl;
				break;
			}
		}
	}
	
	/**
	 * Prints all known variables on the console if the verbosity is at most equal to the currently
	 * set verbosity level. Setting a value closer to ERROR will make the output more likely to pass
	 * the verbosity constraint.
	 *
	 * @param[in] verbosity - output at the verbosity level
	 * @param[in] it - extract script location from this iterator
	 */
	void printAllVariables(const Verbosity verbosity, const Iterator & it) {
		if (verbosity <= this->script.config.verbosity) {
			const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
			const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
			const pp::VariableScopes & varScopes(this->script.vars.getScopes());
			std::string verbosityStr = boost::locale::to_lower(boost::lexical_cast<std::string>(verbosity));
			if ( ! verbosityStr.empty() ) verbosityStr[0] = static_cast<char>(::toupper(verbosityStr[0]));
			std::set<std::string> printed; /* exclude already printed variables */
			std::ostringstream sout;
			BOOST_FOREACH(const pp::VariableMap & map, varScopes) {
				BOOST_FOREACH(const pp::VariableMap::value_type & pair, map) {
					if (printed.find(pair.first) == printed.end()) {
						const boost::optional<pp::StringLiteral &> varVal = this->script.vars.get(pair.first);
						if ( varVal ) {
							printed.insert(pair.first);
							sout << scriptPos << ": " << verbosityStr << ": " << pair.first << " = \"" << varVal->getVarString() << '"' << std::endl;
						}
					}
				}
			}
			switch (verbosity) {
			case VERBOSITY_INFO:
				std::cout << sout.str();
				break;
			case VERBOSITY_ERROR:
				BOOST_THROW_EXCEPTION(
					pcf::exception::Script()
					<< pcf::exception::tag::Message(sout.str())
				);
				break;
			default:
				std::cerr << sout.str();
				break;
			}
		}
	}
	
	/**
	 * Imports the given script file at the current location.
	 *
	 * @param[in] p - path to the requested script file
	 * @return true on success, else false
	 */
	bool readImport(const pp::StringLiteral & p) {
		if (p.isSet() == false || p.isVariable()) return false;
		boost::filesystem::path path(p.getString(), pcf::path::utf8);
		try {
			return this->script.readImport(path, this->sourcePath);
		} catch (const pcf::exception::FileNotFound & e) {
			addLineInfoToException(p.getLineInfo(), e);
		} catch (const pcf::exception::Input & e) {
			addLineInfoToException(p.getLineInfo(), e);
		}
		return false;
	}
	
	/**
	 * Includes the given script file at the current location.
	 *
	 * @param[in] p - path to the requested script file
	 * @return true on success, else false
	 */
	bool readInclude(const pp::StringLiteral & p) {
		if (p.isSet() == false || p.isVariable()) return false;
		boost::filesystem::path path(p.getString(), pcf::path::utf8);
		try {
			return this->script.readInclude(path, this->sourcePath);
		} catch (const pcf::exception::FileNotFound & e) {
			addLineInfoToException(p.getLineInfo(), e);
		} catch (const pcf::exception::Input & e) {
			addLineInfoToException(p.getLineInfo(), e);
		}
		return false;
	}
	
	/**
	 * Returns the numeric representation of the content from the given variable in the desired type.
	 *
	 * @param[out] out - set this variable with the result
	 * @param[in] varName - convert content of this variable
	 * @return true on success, else false
	 * @tparam T - numeric type of the output variable
	 */
	template <typename T>
	bool getVariableAsNum(T & out, const std::string & varName) {
		boost::optional<pp::StringLiteral &> var = this->script.vars.get(varName);
		if (( ! var ) || var->isSet() == false) {
			std::cerr << "Warning: Trying to access unknown variable \"" + varName + "\"." << std::endl;
			VariableScopes vs = this->script.vars.getScopes();
			std::cerr << "Known variables:" << std::endl;
			BOOST_FOREACH(const VariableMap & vm, vs) {
				BOOST_FOREACH(const VariableMap::value_type & e, vm) {
					std::cerr << e.first << " = " << e.second << std::endl;
				}
			}
			return false;
		}
		if ( var->isVariable() ) {
			std::cerr << "Warning: Trying to access unknown reference in \"" + varName + "\"." << std::endl;
			return false;
		}
		try {
			out = boost::lexical_cast<T>(var->getString());
		} catch (...) {
			std::cerr << "Error: Failed to cast " << varName << " to number." << std::endl;
			return false;
		}
		return true;
	}
	
	/**
	 * Removes the given variable from all scopes.
	 *
	 * @param[in] varName - remove this variable
	 * @param[in] it - extract script location from this iterator
	 */
	void unsetVariable(const std::string & varName, const Iterator & it) {
		const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
		const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
		if ( ! this->script.vars.unset(varName) ) {
			if ( this->script.config.variableChecking ) {
				if (this->script.config.verbosity >= VERBOSITY_WARN) {
					std::cerr << scriptPos << " Warning: Trying to remove non-existing variable \"" << varName << "\"." << std::endl;
				}
			} else {
				if (this->script.config.verbosity >= VERBOSITY_INFO) {
					std::cerr << scriptPos << " Info: Trying to remove non-existing variable \"" << varName << "\"." << std::endl;
				}
			}
		}
	}
	
	/**
	 * Sets the given variable with the passed string literal at the current scope.
	 *
	 * @param[in] varName - associate string literal to this variable
	 * @param[in] value - set this value
	 * @param[in] enableChecking - set to enable checking of referenced variable replacement
	 * @param[in] warnOnOverwrite - set to warn if the variable already exists within the current scope
	 * @return true on success, else false
	 */
	bool setVariable(const std::string & varName, const pp::StringLiteral & value, const bool enableChecking, const bool warnOnOverwrite = false) {
		VariableHandler::Checking checkLevel = VariableHandler::CHECKING_OFF;
		boost::optional<VariableMap &> currentScope = this->script.vars.getCurrentScope();
		if ( enableChecking ) {
			if ( this->script.config.variableChecking ) {
				checkLevel = VariableHandler::CHECKING_ERROR;
			} else {
				checkLevel = VariableHandler::CHECKING_WARN;
			}
		}
		if (warnOnOverwrite && currentScope && currentScope->find(varName) != currentScope->end()) {
			std::cerr << value.getLineInfo() << ": Warning: Variable \"" << varName << "\" was already defined and will be replaced by a new value." << std::endl;
		}
		return this->script.vars.set(varName, value, checkLevel).isSet();
	}
	
	/**
	 * Sets the given destination variable to the given value in the desired process block.
	 *
	 * @param[in,out] out - set destination variable in this process block
	 * @param[in] varName - associate string literal to this variable
	 * @param[in] value - set this value
	 * @param[in] isTemporary - set to true to mark this destination file as temporary output
	 * @return true on success, else false
	 */
	bool setDestinationVariable(ProcessBlock & out, const std::string & varName, const pp::StringLiteral & value, const bool isTemporary) {
		pp::StringLiteral replacedValue = this->script.vars.set(varName, value, VariableHandler::CHECKING_OFF);
		const bool returnValue = replacedValue.isSet();
		if ( returnValue ) {
			PathLiteral destinationValue(replacedValue);
			if ( isTemporary) destinationValue.addFlags(PathLiteral::TEMPORARY);
			out.destinations[varName] = destinationValue;
		}
		return returnValue && this->setVariable(varName, value, false);
	}
	
	/**
	 * Sets the given dependency variable to the given value in the desired process block.
	 *
	 * @param[in,out] out - set dependency variable in this process block
	 * @param[in] varName - associate string literal to this variable
	 * @param[in] value - set this value
	 * @return true on success, else false
	 */
	bool setDependencyVariable(ProcessBlock & out, const std::string & varName, const pp::StringLiteral & value) {
		pp::StringLiteral replacedValue = this->script.vars.set(varName, value, VariableHandler::CHECKING_OFF);
		const bool returnValue = replacedValue.isSet();
		if ( returnValue ) {
			PathLiteral dependencyValue(replacedValue);
			out.dependencies[varName] = dependencyValue;
		}
		return returnValue && this->setVariable(varName, value, false);
	}
	
	/**
	 * Sets the referenced command from the given string.
	 * 
	 * @param[in,out] out - update this command
	 * @param[in] cmd - parse this string and set the output variable with the result
	 * @param[in] it - extract script location from this iterator
	 * @return true on success, else false
	 */
	bool setCommand(Command & out, const std::string & cmd, const Iterator & it) {
		const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
		const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
		pp::StringLiteral value;
		value.setString(cmd, scriptPos);
		if ( ! value.isSet() ) return false;
		value.replaceVariables(this->script.vars);
		out.shell = this->script.shells[this->script.config.shell];
		out.command = value;
		return true;
	}
	
	/**
	 * Adds the global variables to local process block scope.
	 * 
	 * @param[out] pb - add global variables to this process block scope
	 */
	void setLocalVariables(ProcessBlock & pb) {
		pb.globalVars = this->script.vars;
	}
	
	/**
	 * Sets the process block type and input file filter from the given values.
	 *
	 * @param[in,out] pb - update this process block
	 * @param[in] t - process block type
	 * @param[in] f - input file filter to use
	 * @param[in] i - set true to invert the meaning of the input file filter
	 */
	static void setProcessBlockInfo(ProcessBlock & pb, const ProcessBlock::Type t, const pp::StringLiteral & f, const bool i) {
		if (f.isVariable() || f.isSet() == false) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::SyntaxError()
				<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(f.getLineInfo()) + ": Invalid input file filter \"" + f.getVarString() + "\".")
			);
		}
		const std::string filter(f.getString());
		const std::wstring filterW(boost::locale::conv::utf_to_utf<wchar_t>(filter));
		try {
			boost::wregex regex(filterW, boost::regex_constants::perl);
		} catch (const boost::regex_error & e) {
			throwRegexException(f, e);
		}
		pb.type = t;
		pb.setFilter(filterW, i);
	}
	
	/**
	 * Enters a new variable scope within the script parser context.
	 */
	void enterScope() {
		this->script.vars.addScope();
	}
	
	/**
	 * Leaves the current variable scope within the script parser context.
	 */
	void leaveScope() {
		this->script.vars.removeScope();
	}
	
	/**
	 * Switches between global and local variable scope.
	 */
	void switchScope() {
		std::swap(this->script.vars, this->script.localVars);
	}
	
	/**
	 * Returns the current variable from from the script parser context.
	 *
	 * @param[out] out - set this variable to the current variable scope
	 * @note The output variable remains unchanged if there is no current variable scope.
	 */
	void getCurrentScope(VariableMap & out) {
		boost::optional<VariableMap &> currentScope(this->script.vars.getCurrentScope());
		if ( currentScope ) {
			out = *currentScope;
		}
	}
	
	/**
	 * Copies a given variable from a different scope to the current scope.
	 *
	 * @param[in] varName - copy this variable
	 * @param[in] enableChecking - set to enable checking of referenced variable replacement
	 * @return true on success, else false
	 */
	bool addVariableToCurrentScope(const std::string & varName, const bool enableChecking) {
		boost::optional<pp::StringLiteral &> var = this->script.vars.get(varName);
		if (( ! var ) || var->isSet() == false) {
			std::cerr << "Warning: Trying to access unknown variable \"" << varName << "\"." << std::endl;
			return false;
		}
		(*(this->script.vars.getCurrentScope()))[varName] = *var;
		return true;
	}
	
	/**
	 * Adds or removes the environment variables from the top-most variable scope.
	 *
	 * @param[in] on - set to true to enable the environment variables, false to disable them
	 */
	void switchEnvironmentVariables(const bool on) {
		VariableScopes & varScope(this->script.vars.getScopes());
		if ( varScope.empty() ) return;
		if ( on ) {
			varScope.front() = this->script.environment;
		} else {
			varScope.front().clear();
		}
	}
	
	/**
	 * Includes the process blocks of a given process ID in the passed process.
	 *
	 * @param[in,out] proc - include process blocks here
	 * @param[in] toInclude - copy all process blocks from here
	 */
	static void includeProcessBlocks(Process & proc, const Process * toInclude) {
		proc.include(*toInclude);
	}
	
	/**
	 * Adds a given process block to the passed process.
	 *
	 * @param[in,out] proc - add to this process
	 * @param[in] pb - add this process block
	 */
	static void addProcessBlock(Process & proc, const ProcessBlock & pb) {
		proc.processBlocks.push_back(pb);
	}
	
	/**
	 * Sets the ID of the passed process.
	 *
	 * @param[in,out] proc - set ID for this process
	 * @param[in] id - set to this ID
	 */
	static void setProcessId(Process & proc, const std::string & id) {
		proc.id = id;
	}
	
	/**
	 * Returns a pointer to a process by its process ID.
	 *
	 * @param[out] proc - return process pointer to this variable
	 * @param[in] id - get process by this ID
	 * @return true if such a process was found, else false
	 */
	bool getProcess(Process * & proc, const std::string & id) {
		ProcessMap::iterator procIt(this->script.processes.find(id));
		if (procIt == this->script.processes.end()) return false;
		proc = &(procIt->second);
		return true;
	}
	
	/**
	 * Adds the given process by its process ID to the list of known processes within the script
	 * context. This copies the current script configuration to the given process.
	 *
	 * @param[in,out] proc - add this process
	 */
	void addProcess(Process & proc) {
		proc.config = this->script.config;
		this->script.processes[proc.id] = proc;
	}
	
	/**
	 * Sets the ID of the passed execution target.
	 *
	 * @param[in,out] exec - set ID for this execution target
	 * @param[in] id - set to this ID
	 */
	static void setExecutionId(Execution & exec, const std::string & id) {
		exec.id = id;
	}
	
	/**
	 * Callback function to set the build flag to true for a given process node.
	 *
	 * @param[in,out] element - set build flag for this process
	 * @param[in] level - hierarchical level of the given process node with the dependency tree
	 * @return true on success, else false
	 */
	static bool setBuildFlag(ProcessNode::ValueType & element, const size_t /* level */) {
		element.process.config.build = true;
		return true;
	}
	
	/**
	 * Sets the given values to the referenced process node.
	 *
	 * @param[in,out] output - set values for this process node
	 * @param[in] id - copy execution with this ID
	 * @param[in] fBuild - set to true to force build of this process node
	 * @return true on success, else false
	 */
	bool setExecutionElement(ProcessNode & output, const std::string & id, const bool fBuild) {
		ExecutionMap::const_iterator target = this->script.targets.find(id);
		if (target == this->script.targets.end()) return false; /* no error; may be a process ID instead */
		output.parallel = target->second.processes;
		if ( fBuild ) {
			/* force build for all nodes */
			output.traverseTopDown(Script::setBuildFlag);
		}
		return true;
	}
	
	/**
	 * Sets the given values to the referenced process element.
	 *
	 * @param[in,out] output - set values for this process element
	 * @param[in] id - copy process with this ID
	 * @param[in] input - list of input file list generators
	 * @param[in] addInitialInput - set to true to add files generated by the input file list
	 * generators additionally to the outputs from the dependencies, false to ignore the outputs
	 * from the dependencies
	 * @param[in] it - extract script location from this iterator
	 * @param[in] vars - resolve the passed variables within the commands of the copied process
	 * @param[in] fBuild - set to true to force build of this process element
	 * @return true on success, else false
	 */
	bool setProcessElement(ProcessElement & output, const std::string & id, const pp::StringLiteralVector & input, const bool addInitialInput, const Iterator & it, const VariableMap & vars, const bool fBuild) {
		ProcessMap::const_iterator proc = this->script.processes.find(id);
		if (proc == this->script.processes.end()) {
			const boost::spirit::classic::file_position_base<std::string> & pos = it.get_position();
			const pp::LineInfo scriptPos(pos.file, static_cast<size_t>(pos.line), static_cast<size_t>(pos.column));
			BOOST_THROW_EXCEPTION(
				pcf::exception::SymbolUnknown()
				<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(scriptPos) + ": Error: Trying to access unknown process \"" + id + "\".")
			);
			return false;
		}
		output.process = proc->second;
		output.process.completeDestinationVariables(VariableHandler(vars));
		output.process.completeCommandVariables(VariableHandler(vars));
		output.process.config.build = output.process.config.build || fBuild;
		output.initialInput = input;
		output.addInitialInput = addInitialInput || input.empty();
		output.input.clear();
		return true;
	}
	
	/**
	 * Add a given process node to the passed execution target.
	 *
	 * @param[in,out] exec - add process node to this execution target
	 * @param[in] node - add this node
	 */
	static void addExecutionProcessNode(Execution & exec, const ProcessNode & node) {
		exec.processes.push_back(node);
	}
	
	/**
	 * Includes the process nodes of a given execution target ID in the passed execution target.
	 *
	 * @param[in,out] exec - include process nodes here
	 * @param[in] toInclude - copy all process nodes from here
	 */
	static void includeExecutionNodes(Execution & exec, const Execution * toInclude) {
		exec.include(*toInclude);
	}
	
	/**
	 * Returns a pointer to a execution target by its execution target ID.
	 *
	 * @param[out] exec - return execution target pointer to this variable
	 * @param[in] id - get execution target by this ID
	 * @return true if such a execution target was found, else false
	 */
	bool getExecution(Execution * & exec, const std::string & id) {
		ExecutionMap::iterator execIt(this->script.targets.find(id));
		if (execIt == this->script.targets.end()) return false;
		exec = &(execIt->second);
		return true;
	}
	
	/**
	 * Adds the given execution target by its execution target ID to the list of known execution
	 * targets within the script context. This copies the current script configuration to the given
	 * execution target. It also opens the database if needed and sets the logging output accordingly.
	 *
	 * @param[in,out] exec - add this execution target
	 */
	void addExecution(Execution & exec) {
		/* set log file */
		const boost::optional<pp::StringLiteral &> logFilePath(this->script.vars.get("log"));
		if ( logFilePath && logFilePath->isSet() ) {
			exec.setLogOutput(boost::filesystem::path(logFilePath->getString(), pcf::path::utf8));
		} else {
			exec.setLogOutput();
		}
		/* set database file */
		if ( this->script.config.removeRemains ) {
			/* database is only active if auto remaining file removal is set */
			const boost::optional<pp::StringLiteral &> dbFilePath(this->script.vars.get("db"));
			if ( dbFilePath && dbFilePath->isSet() ) {
				exec.setDatabase(boost::filesystem::path(dbFilePath->getString(), pcf::path::utf8));
			} else {
				boost::filesystem::path databasePath(this->script.mainSource.parent_path());
				exec.setDatabase(databasePath / (this->script.mainSource.filename().string(pcf::path::utf8) + ".db"));
			}
		}
		/* set configuration */
		exec.config = this->script.config;
		/* add target */
		this->script.targets[exec.id] = exec;
	}
	
	/* process execution chain */
	/**
	 * Sets the leaf process to the given node.
	 *
	 * @param[in,out] root - set leaf value to this node
	 * @param[in] value - set this leaf value
	 */
	static void setLeafProcess(ProcessNode & root, const ProcessElement & value) {
		root.value.reset(value);
	}
	
	/**
	 * Adds the passed node as new root depending on the given root node and returns it.
	 *
	 * @param[in,out] root - depend on this node
	 * @param[in] node - new root node
	 * @return new root node
	 * @remarks This is needed to reverse the dependency order from the intermediately generated
	 * abstract syntax tree in the script parser.
	 */
	static ProcessNode getDepProcessNode(ProcessNode & root, const ProcessNode & node) {
		ProcessNode newRoot = node;
		if (newRoot.parallel.empty() && newRoot.dependency.empty() && ( ! root.value )) {
			newRoot.parallel = root.parallel;
			newRoot.dependency = root.dependency;
		} else {
			newRoot.dependency.push_back(root);
		}
		return newRoot;
	}
	
	/**
	 * Adds a given process node to the list of parallel process nodes within the passed node.
	 * 
	 * @param[in,out] root - add to this node
	 * @param[in] node - add this node
	 */
	static void addProcessNode(ProcessNode & root, const ProcessNode & node) {
		root.value.reset();
		root.parallel.push_back(node);
	}
	
	/**
	 * Replaces the list of parallel process nodes in the root node with the one of the given
	 * process node.
	 *
	 * @param[in,out] root - set parallel process node list of this node
	 * @param[in] node - copy parallel process node list from this node
	 */
	static void setProcessParallel(ProcessNode & root, const ProcessNode & node) {
		root.parallel = node.parallel;
	}
};


} /* namespace parser */
} /* namespace pp */


#endif /* __PP_PARSER_SCRIPT_HPP__ */
