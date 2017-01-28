/**
 * @file Variable.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2017 Daniel Starke
 * @date 2015-01-27
 * @version 2016-12-19
 */
#include <algorithm>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/file/Stream.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/string/Escape.hpp>
#include "Variable.hpp"
#include "parser/StringLiteral.hpp"
#include "parser/Utility.hpp"


namespace std {


/**
 * Less than compare operator.
 *
 * @param[in] lh - left hand side
 * @param[in] rh - right hand side
 * @return true if lh < rh, else false
 */
template <>
bool operator< (const pp::StringLiteralFunctionPair & lh, const pp::StringLiteralFunctionPair & rh) {
	return (lh.first < rh.first);
}


/**
 * Equality compare operator.
 * 
 * @param[in] lh - left hand side
 * @param[in] rh - right hand side
 * @return true if lh == rh, else false
 */
template <>
bool operator== (const pp::StringLiteralFunctionPair & lh, const pp::StringLiteralFunctionPair & rh) {
	return (lh.first == rh.first);
}


} /* namespace std */


namespace pp {


#ifndef _MSC_VER
/**
 * Combines two parsing flags.
 *
 * @param[in] lhs - left hand side
 * @param[in] rhs - right hand side
 * @return both flags together
 */
StringLiteral::ParsingFlags operator| (const StringLiteral::ParsingFlags lhs, const StringLiteral::ParsingFlags rhs) {
	return static_cast<StringLiteral::ParsingFlags>(
		static_cast<int>(lhs) | static_cast<int>(rhs)
	);
}


/**
 * Compares two parsing flags. Right hand side or left hand side
 * should be a single parsing flag.
 *
 * @param[in] lhs - left hand side
 * @param[in] rhs - right hand side
 * @return true if at least one flag matches, else false
 */
bool operator== (const StringLiteral::ParsingFlags lhs, const StringLiteral::ParsingFlags rhs) {
	return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
}
#endif


/**
 * Returns a key/value pair from the given string if parsing was successful.
 *
 * @param[in] str - string to parse
 * @param[in] source - source of the string for syntax error reporting
 * @return the parsed value on success
 */
boost::optional<VariableMap::value_type> getKeyValuePair(const std::string & str, const boost::filesystem::path source) {
	namespace phx = boost::phoenix;
	using boost::spirit::qi::_1;
	using boost::spirit::qi::lit;
	using boost::spirit::qi::as_string;
	using boost::spirit::standard::char_;
	std::string::const_iterator first, last;
	std::string key, value;
	first = str.begin();
	last = str.end();
	const bool result = boost::spirit::qi::parse(
		first,
		last,
		as_string[(char_("_a-zA-Z") >> *char_("_a-zA-Z0-9"))][phx::ref(key) = _1] >> lit("=") >> as_string[(*char_)][phx::ref(value) = _1]
	);
	if (first != last || result == false) {
		return boost::optional<VariableMap::value_type>();
	}
	return boost::optional<VariableMap::value_type>(
		VariableMap::value_type(
			key, StringLiteral(value, LineInfo(source), StringLiteral::RAW)
		)
	);
}


/**
 * Outputs the given line information to the passed output stream.
 *
 * @param[in,out] out - output script location to this output stream
 * @param[in] li - script location to output
 * @return reference to this object for chained operations
 */
std::ostream & operator<< (std::ostream & out, const LineInfo & li) {
	out << li.file.string(pcf::path::utf8) << ":" << li.line << ":" << li.column;
	return out;
}


/**
 * Outputs the given string literal to the passed output stream.
 *
 * @param[in,out] out - output string literal to this output stream
 * @param[in] sl - string literal to output
 * @return reference to this object for chained operations
 */
std::ostream & operator<< (std::ostream & out, const StringLiteral & sl) {
	out << sl.getString();
	return out;
}


/**
 * Returns the matched named capture names from the regular expression.
 *
 * @param[in] pattern - parse this regular expression
 * @return list of parsed capture names
 */
RegExNamedCaptureSet getRegExCaptureNames(const std::string & pattern) {
	namespace phx = boost::phoenix;
	using boost::phoenix::insert;
	using boost::spirit::qi::_1;
	using boost::spirit::qi::lit;
	using boost::spirit::qi::omit;
	using boost::spirit::qi::as_string;
	using boost::spirit::standard::char_;
	RegExNamedCaptureSet result;
	std::string::const_iterator first, last;
	first = pattern.begin();
	last = pattern.end();
	boost::spirit::qi::parse(
		first,
		last,
		*(
			as_string[lit("(?<") >> char_("a-zA-Z") >> *char_("a-zA-Z0-9") >> lit('>')][insert(phx::ref(result), _1)]
			| omit[parser::escapedChar | char_]
		)
	);
	return result;
}


/**
 * Replaces all referenced variables by using the variables of the given variable handler.
 * Dynamic variables are not replaced and the given output variable is set with the name of the
 * variable which was not found for replacement on error.
 * 
 * @param[in,out] parts - replace variables in these string literal parts
 * @param[out] unknownVariable - set with the name of an unknown variable
 * @param[in] varHandler - use these variables for replacement
 * @param[in] dynVars - do not replace these variables
 * @return true on success, else false
 */
bool StringLiteral::replaceVariables(StringLiteralList & parts, std::string & unknownVariable, const VariableHandler & varHandler, const DynamicVariableSet & dynVars) {
	DynamicVariableSet::const_iterator dynNotFound = dynVars.end();
	boost::optional<const StringLiteral &> var;
	bool result = true;
	BOOST_FOREACH(StringLiteralPart & part, parts) {
		switch (part.type) {
		case StringLiteralPart::STRING:
			break; /* no action needed */
		case StringLiteralPart::VARIABLE:
			if (dynVars.find(part.value) != dynNotFound) {
				continue;
			} else if ( (var = varHandler.get(part.value)) ) {
				/* replace variable */
				StringLiteral varLit(*var);
				result = varLit.replaceVariables(unknownVariable, varHandler, dynVars) && result;
				part.type = StringLiteralPart::SUB;
				part.sub.clear();
				BOOST_FOREACH(StringLiteralCapturePair & capture, varLit.literal) {
					part.sub.splice(part.sub.end(), capture.second);
				}
			} else {
				unknownVariable = part.value;
				result = false;
			}
			break;
		case StringLiteralPart::SUB:
			result = StringLiteral::replaceVariables(part.sub, unknownVariable, varHandler, dynVars) && result;
			break;
		}
	}
	return result;
}


/**
 * Replaces all referenced variables by using the variables of the given variable map.
 * Dynamic variables are not replaced and the given output variable is set with the name of the
 * variable which was not found for replacement on error.
 *
 * @param[out] unknownVariable - set with the name of an unknown variable
 * @param[in] vars - use these variables for replacement
 * @param[in] dynVars - do not replace these variables
 * @return true on success, else false
 */
bool StringLiteral::replaceVariables(std::string & unknownVariable, const VariableMap & vars, const DynamicVariableSet & dynVars) {
	const VariableHandler varHandler(vars);
	return this->replaceVariables(unknownVariable, varHandler, dynVars);
}


/**
 * Replaces all referenced variables by using the variables of the given variable map.
 * Dynamic variables are not replaced.
 *
 * @param[in] vars - use these variables for replacement
 * @param[in] dynVars - do not replace these variables
 * @return true on success, else false
 */
bool StringLiteral::replaceVariables(const VariableMap & vars, const DynamicVariableSet & dynVars) {
	std::string ignoreUnkownVarStr;
	return this->replaceVariables(ignoreUnkownVarStr, vars, dynVars);
}


/**
 * Replaces all referenced variables by using the variables of the given variable handler.
 * Dynamic variables are not replaced and the given output variable is set with the name of the
 * variable which was not found for replacement on error.
 *
 * @param[out] unknownVariable - set with the name of an unknown variable
 * @param[in] varHandler - use these variables for replacement
 * @param[in] dynVars - do not replace these variables
 * @return true on success, else false
 */
bool StringLiteral::replaceVariables(std::string & unknownVariable, const VariableHandler & varHandler, const DynamicVariableSet & dynVars) {
	bool result = true;
	/* perfect forward for simple assignments (this ensures that captures are correctly passed through) */
	if (this->literal.size() == 1 && this->literal.front().second.size() == 1) {
		StringLiteralPart & part(this->literal.front().second.front());
		if (part.type == StringLiteralPart::VARIABLE) {
			if ( part.functions.empty() ) {
				boost::optional<const StringLiteral &> var;
				if (dynVars.find(part.value) != dynVars.end()) {
					return true;
				} else if ( (var = varHandler.get(part.value)) ) {
					if ( var ) {
						/* replace whole variable (retain line info) */
						const LineInfo li(this->lineInfo);
						this->operator= (*var);
						this->lineInfo = li;
						return true;
					}
				}
				unknownVariable = part.value;
				return false;
			}
		} else {
			return true;
		}
	}
	BOOST_FOREACH(StringLiteralCapturePair & capture, this->literal) {
		result = StringLiteral::replaceVariables(capture.second, unknownVariable, varHandler, dynVars) && result;
	}
	/* reduce number of string literal parts by combining string parts and resolve variable functions */
	this->fold();
	return result;
}


/**
 * Replaces all referenced variables by using the variables of the given variable map.
 * Dynamic variables are not replaced and the given output variable is set with the name of the
 * variable which was not found for replacement on error.
 *
 * @param[out] unknownVariable - set with the name of an unknown variable
 * @param[in] varHandler - use these variables for replacement
 * @return true on success, else false
 * @remarks Dynamic variables are used as defined in the given variable handler.
 */
bool StringLiteral::replaceVariables(std::string & unknownVariable, const VariableHandler & varHandler) {
	return this->replaceVariables(unknownVariable, varHandler, varHandler.getDynamicVariables());
}


/**
 * Replaces all referenced variables by using the variables of the given variable map.
 * Dynamic variables are not replaced.
 *
 * @param[in] varHandler - use these variables for replacement
 * @param[in] dynVars - do not replace these variables
 * @return true on success, else false
 */
bool StringLiteral::replaceVariables(const VariableHandler & varHandler, const DynamicVariableSet & dynVars) {
	std::string ignoreUnkownVarStr;
	return this->replaceVariables(ignoreUnkownVarStr, varHandler, dynVars);
}


/**
 * Replaces all referenced variables by using the variables of the given variable map.
 * Dynamic variables are not replaced.
 *
 * @param[in] varHandler - use these variables for replacement
 * @return true on success, else false
 * @remarks Dynamic variables are used as defined in the given variable handler.
 */
bool StringLiteral::replaceVariables(const VariableHandler & varHandler) {
	std::string ignoreUnkownVarStr;
	return this->replaceVariables(ignoreUnkownVarStr, varHandler);
}


/**
 * Folds this string literal internally to make comparison possible (normalized AST) and to save memory.
 * 
 * @param[in] final - set to remove all variable references
 * @param[in] dynVars - do not remove these variables
 */
void StringLiteral::fold(const bool final, const DynamicVariableSet & dynVars) {
	BOOST_FOREACH(StringLiteralCapturePair & capture, this->literal) {
		StringLiteral::fold(capture.second, final, dynVars);
	}
}


/**
 * Folds this string literal internally to make comparison possible and to save memory.
 * 
 * @param[in] segment - string literal part list
 * @param[in] final - set to remove all variable references
 * @param[in] dynVars - do not remove these variables
 * @return true if the folded segment is complete (safe to apply functions), false if still variable
 */
bool StringLiteral::fold(StringLiteralList & segment, const bool final, const DynamicVariableSet & dynVars) {
	StringLiteralList::iterator part, endPart;
	boost::optional<StringLiteralList::iterator> lastPart;
	bool result = true;
	for (part = segment.begin(), endPart = segment.end(); part != endPart; ) {
		switch (part->type) {
		case StringLiteralPart::STRING:
			if ( lastPart ) {
				(*lastPart)->value.append(part->value);
				part = segment.erase(part);
				continue;
			} else {
				lastPart.reset(part);
			}
			break;
		case StringLiteralPart::VARIABLE:
			if (final && dynVars.find(part->value) == dynVars.end()) {
				part = segment.erase(part);
				continue;
			} else {
				result = false;
				lastPart.reset();
			}
			break;
		case StringLiteralPart::SUB:
			if ( StringLiteral::fold(part->sub, final, dynVars) ) {
				/* sub segment is complete (should consists of at most a single string) */
				if (part->sub.size() > 1) {
					BOOST_THROW_EXCEPTION(
						pcf::exception::InvalidValue()
						<< pcf::exception::tag::Message("Broken string literal segment.")
					);
					return false;
				}
				std::string value(part->sub.empty() ? std::string() : part->sub.front().value);
				BOOST_FOREACH(const StringLiteralFunctionPair & function, part->functions) {
					if ( function.second ) function.second(value);
				}
				if ( lastPart ) {
					(*lastPart)->value.append(value);
					part = segment.erase(part);
					continue;
				} else {
					*part = StringLiteralPart(value, StringLiteralPart::STRING);
					lastPart.reset(part);
				}
			} else {
				result = false;
				lastPart.reset();
			}
			break;
		}
		++part;
	}
	return result;
}


/**
 * Sets the string literal value from the given string by parsing it accordingly.
 *
 * @param[in] str - string to parse
 * @param[in] parsingFlags - parsing options
 */
void StringLiteral::setLiteralFromString(const std::string & str, const StringLiteral::ParsingFlags parsingFlags) {
	/* reduce used iterator types by using the same as in pp::Script::readInclude() */
	typedef boost::spirit::istream_iterator IteratorType;
	typedef boost::spirit::classic::position_iterator2<IteratorType> PosIteratorType;
	std::istringstream inputStream(str);
	inputStream.unsetf(std::ios::skipws);
	IteratorType begin(inputStream), end;
	PosIteratorType posBegin(begin, end, "string"), posEnd;
	posBegin.set_tab_chars(1);
	posEnd.set_tab_chars(1);
	
	std::string key, value;
	parser::StringLiteral<PosIteratorType> stringLiteralGrammar(parsingFlags, boost::optional<char>(), true, false);
	StringLiteralCaptureVector attribute;
	bool result = boost::spirit::qi::parse(
		posBegin,
		posEnd,
		stringLiteralGrammar(parsingFlags, boost::phoenix::construct<boost::optional<char> >(), false),
		attribute
	);
	if (posBegin != posEnd || result == false) {
		/* variable is not set */
		this->literal.clear();
		this->set = false;
		this->regexCaptures.clear();
	} else {
		/* set variable */
		this->literal = attribute;
		this->set = true;
		this->regexCaptures.clear();
	}
}


/**
 * @verbatim Replacement function which turns / into \.@endverbatim
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionWin(std::string & str) {
	std::replace(str.begin(), str.end(), '/', '\\');
}


/**
 * @verbatim Replacement function which turns \ into /.@endverbatim
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionUnix(std::string & str) {
	std::replace(str.begin(), str.end(), '\\', '/');
}



/**
 * @verbatim Replacement function which turns \ into \\ and " into \".@endverbatim
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionEsc(std::string & str) {
	str = pcf::string::escapeCharacters(str, '\\', "\\\"");
}


/**
 * Replacement function turns all lower case characters into upper case characters.
 * 
 * @param[in,out] str - apply function on this string
 * @remarks Unicode characters are handled correctly (UTF-8 encoding).
 */
void StringLiteral::functionUpper(std::string & str) {
	str = boost::locale::to_upper(str);
}


/**
 * Replacement function turns all upper case characters into lower case characters.
 * 
 * @param[in,out] str - apply function on this string
 * @remarks Unicode characters are handled correctly (UTF-8 encoding).
 */
void StringLiteral::functionLower(std::string & str) {
	str = boost::locale::to_lower(str);
}


/**
 * Replacement function to find and replace a string.
 *
 * @param[in,out] str - apply function on this string
 * @param[in] replace - replacement description
 */
void StringLiteral::functionReplace(std::string & str, const StringLiteralReplacementPair & replace) {
	using boost::locale::conv::utf_to_utf;
	try {
		boost::wregex search(utf_to_utf<wchar_t>(replace.first.getString()), boost::regex_constants::perl);
		std::wstring wStr(utf_to_utf<wchar_t>(str));
		str = utf_to_utf<char>(
			boost::regex_replace(wStr, search, utf_to_utf<wchar_t>(replace.second.getString()), boost::regex_constants::format_all)
		);
	} catch (const boost::regex_error & e) {
		parser::throwRegexException(replace.first, e);
		return;
	}
}


/**
 * Replacement function to extract a sub string.
 * 
 * @param[in,out] str - apply function on this string
 * @param[in,out] start - start position of the substring
 * @param[in,out] len - length of the substring in characters
 * @remarks Unicode characters are handled correctly (UTF-8 encoding).
 */
void StringLiteral::functionSubstr(std::string & str, const int start, const boost::optional<int> & len) {
	std::wstring wStr(boost::locale::conv::utf_to_utf<wchar_t>(str));
	size_t sStart, sLen = std::wstring::npos;
	if ( len ) {
		int nStart;
		int nLen = *len;
		if (nLen < 0) {
			if (start < 0) {
				nStart = static_cast<int>(wStr.size()) + start + nLen;
			} else {
				nStart = start + nLen;
			}
			if (nStart < 0) {
				sStart = 0;
				nLen -= nStart;
				if (nLen > 0) nLen = 0;
			} else {
				sStart = static_cast<size_t>(nStart);
			}
			sLen = static_cast<size_t>(-nLen);
		} else {
			if (start < 0) {
				nStart = static_cast<int>(wStr.size()) + start;
				if (nStart < 0) {
					sStart = 0;
					nLen += static_cast<int>(wStr.size()) + start;
					if (nLen < 0) nLen = 0;
				} else {
					sStart = static_cast<size_t>(nStart);
				}
			} else {
				sStart = static_cast<size_t>(start);
			}
			sLen = static_cast<size_t>(nLen);
		}
	} else {
		if (start < 0) {
			if ((-start) > static_cast<int>(wStr.size())) {
				sStart = 0;
			} else {
				sStart = static_cast<size_t>(static_cast<int>(wStr.size()) + start);
			}
		} else {
			sStart = static_cast<size_t>(start);
		}
	}
	if (sStart > wStr.size() || sLen == 0) {
		/* output empty string if start is out of range or length is zero */
		str.clear();
	} else {
		str = boost::locale::conv::utf_to_utf<char>(wStr.substr(sStart, sLen));
	}
}


/**
 * Replacement function to return the parent directory of the given path.
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionDirectory(std::string & str) {
	str = boost::filesystem::path(str, pcf::path::utf8).parent_path().string(pcf::path::utf8);
}


/**
 * Replacement function to return the file name including the file extension of the given path.
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionFilename(std::string & str) {
	str = boost::filesystem::path(str, pcf::path::utf8).filename().string(pcf::path::utf8);
}


/**
 * Replacement function to return the file name of the given path.
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionFile(std::string & str) {
	str = boost::filesystem::path(str, pcf::path::utf8).stem().string(pcf::path::utf8);
}


/**
 * Replacement function to return the file extension including the dot of the given path.
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionExtension(std::string & str) {
	str = boost::filesystem::path(str, pcf::path::utf8).extension().string(pcf::path::utf8);
}


/**
 * Replacement function to return true if the given path exists or false else.
 * 
 * @param[in,out] str - apply function on this string
 */
void StringLiteral::functionExists(std::string & str) {
	str = boost::filesystem::exists(boost::filesystem::path(str, pcf::path::utf8)) ? "true" : "false";
}


/**
 * Replacement function to return true if the given path exists using regular expressions or false else.
 * 
 * @param[in,out] str - apply function on this string
 * @param[in] li - line information (position where this function was called)
 * @param[in] fullRecursiveMatch - set true to match the whole path, false to match only path elements
 */
void StringLiteral::functionRExists(std::string & str, const LineInfo & li, const bool fullRecursiveMatch) {
	std::vector<boost::filesystem::path> pathList;
	const std::wstring inRegExW(boost::locale::conv::utf_to_utf<wchar_t>(str));
	try {
		boost::wregex regex(
			inRegExW,
			boost::regex_constants::perl
#if defined(PCF_IS_WIN)
			| boost::regex_constants::icase
#endif /* PCF_IS_WIN */
		);
		try {
			if ( ! pcf::path::getRegexPathList(pathList, inRegExW, fullRecursiveMatch) ) {
				str = "false";
				return;
			}
		} catch (const boost::regex_error & e) {
			std::ostringstream sout;
			sout << li << ": Error: Regular expression '" << str << "' is invalid.\n" << e.what();
			BOOST_THROW_EXCEPTION(
				pcf::exception::SyntaxError()
				<< pcf::exception::tag::Message(sout.str())
			);
			str = "false";
			return;
		} catch (...) {
			std::ostringstream sout;
			sout << li << ": Error: Failed to get path list from regular expression \"" << str << "\".";
			BOOST_THROW_EXCEPTION(
				pcf::exception::File()
				<< pcf::exception::tag::Message(sout.str())
			);
			str = "false";
			return;
		}
		
	} catch (const boost::regex_error & e) {
		std::ostringstream sout;
		sout << li << ": Error: Regular expression '" << str << "' is invalid.\n" << e.what();
		BOOST_THROW_EXCEPTION(
			pcf::exception::SyntaxError()
			<< pcf::exception::tag::Message(sout.str())
		);
		str = "false";
		return;
	}
	str = pathList.empty() ? "false" : "true";
}


/**
 * Adds the given flags to the path literal.
 *
 * @param[in] val - flags to add
 * @return reference to this object for chained operations
 */
PathLiteral & PathLiteral::addFlags(const PathLiteral::Flag val) {
	this->flags = this->flags | val;
	return *this;
}


/**
 * Removes the given flags from the path literal.
 *
 * @param[in] val - flags to remove
 * @return reference to this object for chained operations
 */
PathLiteral & PathLiteral::removeFlags(const PathLiteral::Flag val) {
	this->flags = static_cast<PathLiteral::Flag>(static_cast<int>(this->flags) & (~static_cast<int>(val)));
	return *this;
}


#ifndef _MSC_VER
/**
 * Combines two path literal flags.
 *
 * @param[in] lhs - left hand side
 * @param[in] rhs - right hand side
 * @return both flags together
 */
PathLiteral::Flag operator| (const PathLiteral::Flag lhs, const PathLiteral::Flag rhs) {
	return static_cast<PathLiteral::Flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
}


/**
 * Returns true if the given flags match, else false.
 *
 * @param[in] lhs - left hand side
 * @param[in] rhs - right hand side
 * @return true if rhs is included in lhs, else false
 */
bool operator& (const PathLiteral::Flag lhs, const PathLiteral::Flag rhs) {
	return (static_cast<int>(lhs) & static_cast<int>(rhs)) == static_cast<int>(rhs);
}
#endif


} /* namespace pftp */
