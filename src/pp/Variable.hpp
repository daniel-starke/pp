/**
 * @file Variable.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-01-27
 * @version 2016-05-01
 */
#ifndef __PP_VARIABLE_HPP__
#define __PP_VARIABLE_HPP__


#include <cstdlib>
#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/path/Utility.hpp>


namespace pp {
	

/* forward class declarations */
struct LineInfo;
class StringLiteral;
class PathLiteral;
struct LessPathLiteralPtrValue;
class VariableHandler;


/* type definitions */
typedef std::vector<PathLiteral> PathLiteralVector;
typedef std::vector< boost::shared_ptr<PathLiteral> > PathLiteralPtrVector;
typedef std::set< boost::shared_ptr<PathLiteral> > PathLiteralPtrSet;
/** @remarks It needs to be ensured that the key is never changed. Doing so leads to undefined behavior. */
typedef std::map<boost::shared_ptr<PathLiteral>, std::set<boost::filesystem::path>, LessPathLiteralPtrValue> PathLiteralPtrDependentMap;
typedef std::vector<StringLiteral> StringLiteralVector;
typedef std::map<std::string, StringLiteral> VariableMap;
typedef std::map<std::string, PathLiteral> PathVariableMap;
typedef std::set<std::string> DynamicVariableSet;
typedef std::set<std::string> RegExNamedCaptureSet;
typedef std::map<std::string, std::string> DynamicVariableMap;
typedef std::vector<VariableMap> VariableScopes;


/* functions */
/**
 *
 */
boost::optional<VariableMap::value_type> getKeyValuePair(const std::string & str, const boost::filesystem::path source);
std::ostream & operator <<(std::ostream & out, const LineInfo & li);
std::ostream & operator <<(std::ostream & out, const StringLiteral & sl);
RegExNamedCaptureSet getRegExCaptureNames(const std::string & pattern);


/* classes */
/**
 * Structure to store a script location.
 */
struct LineInfo {
	friend std::ostream & operator <<(std::ostream & out, const LineInfo & li);
	boost::filesystem::path file; /**< File path. */
	size_t line; /**< Line within the file (first line is 1). */
	size_t column; /**< Column within the line (first column is 0). */
	
	/**
	 * Default constructor.
	 */
	explicit LineInfo() :
		line(0),
		column(0)
	{};
	
	/**
	 * Constructor.
	 *
	 * @param[in] f - file path
	 * @param[in] l - line
	 * @param[in] c - column
	 */
	explicit LineInfo(const boost::filesystem::path & f, const size_t l = 0, const size_t c = 0) :
		file(f),
		line(l),
		column(c)
	{}
};


/**
 * Type of a replacement pair.
 */
typedef std::pair<StringLiteral, StringLiteral> StringLiteralReplacementPair;


/**
 * Type of a variable function.
 *
 * @param[in,out] str - apply function on this string
 */
typedef boost::function1<void, std::string &> StringLiteralFunction;


/**
 * Type of a function string representation and callback function pair.
 */
typedef std::pair<std::string, StringLiteralFunction> StringLiteralFunctionPair;


} /* namespace pp */
namespace std {


template <>
bool operator< (const pp::StringLiteralFunctionPair & lh, const pp::StringLiteralFunctionPair & rh);


template <>
bool operator== (const pp::StringLiteralFunctionPair & lh, const pp::StringLiteralFunctionPair & rh);


} /* namespace std */
namespace pp {


/**
 * List of variable replacement functions.
 */
typedef std::vector<StringLiteralFunctionPair> StringLiteralFunctionVector;


/**
 * Structure of a string literal segment.
 */
struct StringLiteralPart : boost::totally_ordered<StringLiteralPart> {
	/**
	 * Possible string literal segment types.
	 */
	enum Type {
		STRING, /**< Segment is a standard string. */
		VARIABLE /**< Segment is a reference to a variable. */
	};
	std::string value; /**< String value of the segment. */
	Type type; /**< Type of the segment. */
	StringLiteralFunctionVector functions; /**< Functions to apply to the referenced variable content beforehand. */
	
	/**
	 * Default constructor.
	 */
	explicit StringLiteralPart() :
		value(),
		type(STRING)
	{}
	
	/**
	 * Constructor.
	 *
	 * @param[in] v - string value
	 * @param[in] t - segment type
	 */
	explicit StringLiteralPart(const std::string & v, const Type t) :
		value(v),
		type(t)
	{}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	StringLiteralPart(const StringLiteralPart & o) :
		value(o.value),
		type(o.type),
		functions(o.functions)
	{}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 */
	StringLiteralPart & operator =(const StringLiteralPart & o) {
		if (this != &o) {
			this->value = o.value;
			this->type = o.type;
			this->functions = o.functions;
		}
		return *this;
	}
	
	/**
	 * Equality operator.
	 *
	 * @param[in] rh - right hand side
	 * @return true if type, value and function list match, else false
	 */
	bool operator== (const StringLiteralPart & rh) const {
		if (this->type != rh.type) return false;
		if (this->value != rh.value) return false;
		return (this->functions == rh.functions);
	}
	
	/**
	 * Less than operator.
	 *
	 * @param[in] rh - right hand side
	 * @return true if type, value or functions are less than the passed object, else false
	 */
	bool operator< (const StringLiteralPart & rh) const {
		if (static_cast<int>(this->type) < static_cast<int>(rh.type)) return true;
		if (this->value < rh.value) return true;
		if (this->functions < rh.functions) return true;
		return false;
	}
};


/**
 * Type for a list of string literal segments.
 */
typedef std::list<StringLiteralPart> StringLiteralList;


/**
 * Type for a list of regular expression capture names.
 */
typedef std::vector<std::string> CaptureNameVector;


/**
 * Type for a pair of capture names and string literal segments.
 */
typedef std::pair<CaptureNameVector, StringLiteralList> StringLiteralCapturePair;


/**
 * Type for a list of capture/segment pairs.
 */
typedef std::vector<StringLiteralCapturePair> StringLiteralCaptureVector;


/**
 * Class to handle a string literal.
 */
class StringLiteral : boost::totally_ordered<StringLiteral> {
public:
	/** Possible parsing flags. */
	enum ParsingFlags {
		/* basic type flag */
		STANDARD        = 0x01, /**< Parse string in standard manner; cannot be combined with RAW or ENABLE_CAPTURES. */
		RAW             = 0x02, /**< Take string as it is and don't parse anything; cannot be combined with STANDARD or ENABLE_CAPTURES. */
		ENABLE_CAPTURES = 0x04, /**< Parse including captures; cannot be combined with STANDARD or RAW. */
		/* modifier flag */
		NO_ESCAPE       = 0x08 /**< Additional flag to disable character escape parsing. */
	};
private:
	LineInfo lineInfo; /**< Script file location where this string literal was defined. */
	StringLiteralCaptureVector literal; /**< This string literal. */
	bool set; /**< True if set, else false. */
	/* additional attributes */
	VariableMap regexCaptures; /**< Regular expression captures. */
public:
	/**
	 * Default constructor.
	 */
	explicit StringLiteral():
		lineInfo(),
		literal(),
		set(false)
	{}

	/**
	 * Constructor.
	 *
	 * @param[in] l - string literal as raw string
	 * @param[in] li - script location of the string literal definition
	 * @param[in] pf - parsing flags
	 */
	explicit StringLiteral(const std::string & l, const LineInfo & li, const ParsingFlags pf = STANDARD):
		lineInfo(li)
	{
		this->setLiteralFromString(l, pf);
		this->regexCaptures.clear();
	}
	
	/**
	 * Constructor.
	 *
	 * @param[in] slcv - raw data structure to create the string literal from
	 * @param[in] li - script location of the string literal definition
	 * @param[in] is - true if string literal is set, else false
	 */
	explicit StringLiteral(const StringLiteralCaptureVector & slcv, const LineInfo & li, const bool is = true):
		lineInfo(li),
		literal(slcv),
		set(is)
	{
		this->regexCaptures.clear();
	}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	StringLiteral(const StringLiteral & o):
		lineInfo(o.lineInfo),
		literal(o.literal),
		set(o.set),
		regexCaptures(o.regexCaptures)
	{}
	
	/** 
	 * Returns the script file location where this string literal was defined.
	 *
	 * @return script file location
	 */
	const LineInfo & getLineInfo() const {
		return this->lineInfo;
	}
	
	/**
	 * Sets the location and script file where this string literal was defined.
	 *
	 * @param[in] li - string file location to set
	 * @return reference to this object for chained operations
	 */
	StringLiteral & setLineInfo(const LineInfo & li) {
		this->lineInfo = li;
		return *this;
	}
	
	/**
	 * Returns whether this string literal is set or not.
	 *
	 * @return true if set, else false
	 */
	bool isSet() const {
		return this->set;
	}
	
	/**
	 * Returns whether this string literal is not set or empty.
	 *
	 * @return true if not set or empty, else false
	 */
	bool isEmpty() const {
		if ( this->set ) return true;
		BOOST_FOREACH(const StringLiteralCapturePair & capture, this->literal) {
			BOOST_FOREACH(const StringLiteralPart & part, capture.second) {
				if ( ! part.value.empty() ) return false;
			}
		}
		return true;
	}
	
	/**
	 * Returns true if there are still unresolved variables within this string literal.
	 *
	 * @return true if still variable/dynamic, else false
	 */
	bool isVariable() const {
		if ( ! this->set ) return false;
		BOOST_FOREACH(const StringLiteralCapturePair & capture, this->literal) {
			BOOST_FOREACH(const StringLiteralPart & part, capture.second) {
				if (part.type == StringLiteralPart::VARIABLE) return true;
			}
		}
		return false;
	}
	
	/**
	 * Returns the raw string representation of this string literal.
	 *
	 * @return string representation of the string literal
	 * @note referenced variables are ignored
	 */
	std::string getString() const {
		if ( ! this->set ) return std::string();
		std::string result;
		BOOST_FOREACH(const StringLiteralCapturePair & capture, this->literal) {
			BOOST_FOREACH(const StringLiteralPart & part, capture.second) {
				if (part.type == StringLiteralPart::STRING) result.append(part.value);
			}
		}
		return result;
	}
	
	/**
	 * Returns the raw string representation of this string literal in its definition form.
	 *
	 * @return string representation of the string literal
	 * @note referenced variables are returned in reference style (example: "{var}")
	 */
	std::string getVarString() const {
		if ( ! this->set ) return std::string();
		std::string result;
		BOOST_FOREACH(const StringLiteralCapturePair & capture, this->literal) {
			if ( ! capture.first.empty() ) {
				result.push_back('(');
			}
			BOOST_FOREACH(const StringLiteralPart & part, capture.second) {
				switch (part.type) {
				case StringLiteralPart::VARIABLE:
					result.push_back('{');
					result.append(part.value);
					BOOST_FOREACH(const StringLiteralFunctionPair & function, part.functions) {
						result.push_back(':');
						result.append(function.first);
					}
					result.push_back('}');
					break;
				case StringLiteralPart::STRING:
					result.append(part.value);
					break;
				default:
					break;
				}
			}
			if ( ! capture.first.empty() ) {
				result.push_back(')');
			}
		}
		return result;
	}
	
	/**
	 * Returns a map of associated regular expression captures.
	 *
	 * @return capture map
	 */
	VariableMap getCaptures() const {
		if ( ! this->set ) return VariableMap();
		if ( ! this->regexCaptures.empty() ) return this->regexCaptures;
		VariableMap result;
		StringLiteralList wholeLiteralPart;
		StringLiteral wholeLiteral;
		CaptureNameVector newCapture, wholeCapture;
		wholeCapture.push_back("0");
		BOOST_FOREACH(const StringLiteralCapturePair & capture, this->literal) {
			StringLiteralList literalPart;
			StringLiteral newLiteral;
			BOOST_FOREACH(const StringLiteralPart & part, capture.second) {
				if (part.type == StringLiteralPart::STRING) {
					literalPart.push_back(part);
					wholeLiteralPart.push_back(part);
				}
			}
			newLiteral.literal.clear();
			newLiteral.literal.push_back(StringLiteralCapturePair(newCapture, literalPart));
			newLiteral.set = true;
			BOOST_FOREACH(const std::string & name, capture.first) {
				result[name] = newLiteral;
			}
		}
		wholeLiteral.literal.clear();
		wholeLiteral.literal.push_back(StringLiteralCapturePair(wholeCapture, wholeLiteralPart));
		wholeLiteral.set = true;
		BOOST_FOREACH(const std::string & name, wholeCapture) {
			result[name] = wholeLiteral;
		}
		return result;
	}
	
	/** 
	 * Associates regular expression captures to this string literal.
	 *
	 * @param[in] regex - capture map to associate
	 * @return reference to this object for chained operations
	 */
	StringLiteral & setRegexCaptures(const VariableMap & regex) {
		this->regexCaptures = regex;
		return *this;
	}
	
	/**
	 * Returns the raw string representation of this string literal.
	 *
	 * @return string representation of the string literal
	 * @note referenced variables are ignored
	 */
	operator std::string() const {
		return this->getString();
	}
	
	/**
	 * Sets the string literal by parsing the passed raw string.
	 *
	 * @param[in] str - string to parse
	 * @param[in] enableCaptures - set to true to enable capture parsing, false for standard parsing
	 * @return reference to this object for chained operations
	 */
	StringLiteral & setString(const std::string & str, const bool enableCaptures = false) {
		this->setLiteralFromString(str, enableCaptures ? ENABLE_CAPTURES : STANDARD);
		return *this;
	}
	
	/**
	 * Sets the string literal from a raw string without parsing the string for referenced variables
	 * or captures.
	 *
	 * @param[in] str - string to set
	 * @return reference to this object for chained operations
	 */
	StringLiteral & setRawString(const std::string & str) {
		this->setLiteralFromString(str, RAW);
		return *this;
	}
	
	/**
	 * Sets the string literal by parsing the passed raw string.
	 *
	 * @param[in] str - string to parse
	 * @param[in] li - script file location
	 * @param[in] enableCaptures - set to true to enable capture parsing, false for standard parsing
	 * @return reference to this object for chained operations
	 */
	StringLiteral & setString(const std::string & str, const LineInfo & li, const bool enableCaptures = false) {
		this->lineInfo = li;
		this->setLiteralFromString(str, enableCaptures ? ENABLE_CAPTURES : STANDARD);
		return *this;
	}
	
	/**
	 * Sets the string literal from a raw string without parsing the string for referenced variables
	 * or captures.
	 *
	 * @param[in] str - string to set
	 * @param[in] li - script file location
	 * @return reference to this object for chained operations
	 */
	StringLiteral & setRawString(const std::string & str, const LineInfo & li) {
		this->lineInfo = li;
		this->setLiteralFromString(str, RAW);
		return *this;
	}
	
	/**
	 * Checks whether the string literal references one or more of the given dynamic variables.
	 *
	 * @param[in] dynVars - check against these variables
	 * @return true if there is a reference to such a dynamic variable, else false
	 */
	bool hasDynVariable(const DynamicVariableSet & dynVars) const {
		if ( ! this->set ) return false;
		if ( dynVars.empty() ) return false;
		BOOST_FOREACH(const StringLiteralCapturePair & capture, this->literal) {
			BOOST_FOREACH(const StringLiteralPart & part, capture.second) {
				if (part.type == StringLiteralPart::VARIABLE && dynVars.count(part.value) >= 1) {
					return true;
				}
			}
		}
		return false;
	}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 * @return reference to this object for chained operations
	 */
	StringLiteral & operator= (const StringLiteral & o) {
		if (this != &o) {
			this->lineInfo = o.lineInfo;
			this->literal = o.literal;
			this->set = o.set;
			this->regexCaptures = o.regexCaptures;
		}
		return *this;
	}
	
	/** 
	 * Assignment operator.
	 *
	 * @param[in] str - string to assign
	 * @return reference to this object for chained operations
	 */
	StringLiteral & operator= (const std::string & str) {
		this->setLiteralFromString(str);
		return *this;
	}
	
	/**
	 * Equality comparison operator.
	 *
	 * @param[in] rh - right hand side
	 * @return true if both string literals are equal, else false
	 * @note The flatten() method should be called beforehand to ensure that both string literals
	 * are really comparable.
	 */
	bool operator== (const StringLiteral & rh) const {
		if ( ! this->set ) {
			if ( ! rh.set ) {
				return true;
			} else {
				return false;
			}
		}
		if ( ! rh.set ) return false;
		if (this->regexCaptures != rh.regexCaptures) return false;
		return (this->literal == rh.literal);
	}
	
	/**
	 * Less than comparison operator.
	 *
	 * @param[in] rhs - right hand side
	 * @return true if this string literal shall be ordered before the right hand side one, else false
	 */
	bool operator< (const StringLiteral & rhs) const {
		if (this->set == false && rhs.set == true) return true;
		if (this->set == false || rhs.set == false) return false;
		if (this->regexCaptures < rhs.regexCaptures) return true;
		if (this->literal < rhs.literal) return true;
		return false;
	}
	
	/*
	 * This method can be used to replace the dynamic variables.
	 *
	 * @return true if complete string, else false
	 */
	bool replaceVariables(std::string & unknownVariable, const VariableMap & vars, const DynamicVariableSet & dynVars = DynamicVariableSet());
	
	/*
	 * This method can be used to replace the dynamic variables.
	 * @return true if complete string, else false
	 */
	bool replaceVariables(const VariableMap & vars, const DynamicVariableSet & dynVars = DynamicVariableSet());
	
	/*
	 * @return true if complete string, else false
	 */
	bool replaceVariables(std::string & unknownVariable, const VariableHandler & varHandler, const DynamicVariableSet & dynVars);
	
	/*
	 * @return true if complete string, else false
	 */
	bool replaceVariables(const VariableHandler & varHandler, const DynamicVariableSet & dynVars);
	
	/*
	 * @return true if complete string, else false
	 */
	bool replaceVariables(std::string & unknownVariable, const VariableHandler & varHandler);
	
	/*
	 * @return true if complete string, else false
	 */
	bool replaceVariables(const VariableHandler & varHandler);
	
	/**
	 * Returns a flat representation of this string literal.
	 *
	 * @return flat vector list
	 */
	StringLiteralList getFlatVector() const {
		StringLiteralList result;
		StringLiteralPart::Type lastType = StringLiteralPart::VARIABLE;
		BOOST_FOREACH(const StringLiteralCapturePair & capture, this->literal) {
			BOOST_FOREACH(const StringLiteralPart & part, capture.second) {
				if (lastType == part.type && part.type == StringLiteralPart::STRING) {
					/* combine string parts */
					result.back().value.append(part.value);
				} else {
					result.push_back(part);
				}
				lastType = part.type;
			}
		}
		return result;
	}
	
	void flatten();
	
	StringLiteralList getFlatVectorForReplacement(const StringLiteralFunctionVector & functions, const bool canBeVariable) const;
	static void functionWin(std::string & str);
	static void functionUnix(std::string & str);
	static void functionEsc(std::string & str);
	static void functionUpper(std::string & str);
	static void functionLower(std::string & str);
	static void functionReplace(std::string & str, const StringLiteralReplacementPair & replace);
	static void functionSubstr(std::string & str, const int start, const boost::optional<int> & len);
	static void functionDirectory(std::string & str);
	static void functionFilename(std::string & str);
	static void functionFile(std::string & str);
	static void functionExtension(std::string & str);
	static void functionExists(std::string & str);
private:
	void setLiteralFromString(const std::string & str, const ParsingFlags parsingFlags = STANDARD);
};


StringLiteral::ParsingFlags operator| (const StringLiteral::ParsingFlags lhs, const StringLiteral::ParsingFlags rhs);
bool operator== (const StringLiteral::ParsingFlags lhs, const StringLiteral::ParsingFlags rhs);


/**
 * Specialized string literal class for paths.
 */
class PathLiteral : public StringLiteral, boost::totally_ordered<PathLiteral> {
public:
	/** Possible path flags. */
	enum Flag {
		NONE = 0x00, /**< No flag given. */
		PERMANENT = 0x01, /**< The file is permanent. */
		TEMPORARY = 0x02, /**< The file is temporary. */
		MODIFIED = 0x04, /**< The file was modified. */
		FORCED = 0x08, /**< Forced output by build flags (also for process-wise build flags). */
		EXISTS = 0x10 /**< The file path exists. */
	};
private:
	Flag flags; /**< Path literal flags. */
	boost::posix_time::ptime lastModification; /**< Last modification time of the path. */
public:
	/**
	 * Default constructor.
	 */
	explicit PathLiteral():
		StringLiteral(),
		flags(NONE)
	{}

	/**
	 * Constructor.
	 *
	 * @param[in] l - raw string to create the path literal from
	 * @param[in] li - script file location where the path literal was defined
	 * @param[in] pf - parsing flags
	 */
	explicit PathLiteral(const std::string & l, const LineInfo & li, const StringLiteral::ParsingFlags pf = StringLiteral::STANDARD):
		StringLiteral(l, li, pf),
		flags(NONE)
	{}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - string literal to copy
	 */
	PathLiteral(const StringLiteral & o):
		StringLiteral(o),
		flags(NONE)
	{}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	PathLiteral(const PathLiteral & o):
		StringLiteral(o),
		flags(o.flags),
		lastModification(o.lastModification)
	{}
	
	/**
	 * Returns the path literal flags.
	 *
	 * @return path literal flags
	 */
	Flag getFlags() const {
		return this->flags;
	}
	
	/**
	 * Checks whether the path literal has a set of flags or not.
	 *
	 * @param[in] val - check if these flags are set
	 * @return true if all checked flags are set, else false
	 */
	bool hasFlags(const Flag val) const {
		return this->flags & val;
	}
	
	/**
	 * Sets the given flags as new flags for this path literal.
	 *
	 * @param[in] val - flags to set
	 * @return reference to this object for chained operations
	 */
	PathLiteral & setFlags(const Flag val) {
		this->flags = val;
		return *this;
	}
	
	PathLiteral & addFlags(const Flag val);
	PathLiteral & removeFlags(const Flag val);
	
	/**
	 * Returns the last modification time of the referenced file system object.
	 *
	 * @return modification time
	 */
	boost::posix_time::ptime getLastModification() const {
		return this->lastModification;
	}
	
	/** 
	 * Sets the modification time for the referenced file system object.
	 *
	 * @param[in] lm - last modification time to set
	 * @return reference to this object for chained operations
	 * @remarks This does not change the modification time on the file system.
	 */
	PathLiteral & setLastModification(const boost::posix_time::ptime & lm) {
		this->lastModification = lm;
		return *this;
	}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - string literal to assign
	 * @return reference to this object for chained operations
	 */
	PathLiteral & operator= (const StringLiteral & o) {
		if (this != &o) {
			this->StringLiteral::operator =(o);
		}
		return *this;
	}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 * @return reference to this object for chained operations
	 */
	PathLiteral & operator= (const PathLiteral & o) {
		if (this != &o) {
			this->StringLiteral::operator =(o);
			this->flags = o.flags;
			this->lastModification = o.lastModification;
		}
		return *this;
	}
	
	/**
	 * Equality comparison operator.
	 *
	 * @param[in] rh - right hand side
	 * @return true if both path literals are equal, else false.
	 */
	bool operator== (const PathLiteral & rh) const {
		if ( ! this->StringLiteral::operator ==(rh) ) return false;
		if (this->flags != rh.flags) return false;
		return (this->lastModification == rh.lastModification);
	}
	
	/**
	 * Less than comparison operator.
	 *
	 * @param[in] rh - right hand side
	 * @return true if this path literal should be ordered before the right hand side value, else false
	 */
	bool operator< (const PathLiteral & rh) const {
		if ( this->StringLiteral::operator <(rh) ) return true;
		if (this->flags < rh.flags) return true;
		if (this->lastModification < rh.lastModification) return true;
		return false;
	}
};


/**
 * Functor for less than comparison of two path literal pointers.
 */
struct LessPathLiteralPtrValue {
	/**
	 * Functor for less than comparison of two path literal pointers.
	 *
	 * @param[in] lh - left hand side
	 * @param[in] rh - right hand side
	 * @return true if the pointed left hand side value should be ordered before the pointed
	 * right hand side value, else false
	 */
	bool operator() (const boost::shared_ptr<PathLiteral> & lh, const boost::shared_ptr<PathLiteral> & rh) const {
        return lh->StringLiteral::operator< (static_cast<const StringLiteral &>(*rh));
    }
};


PathLiteral::Flag operator| (const PathLiteral::Flag lhs, const PathLiteral::Flag rhs);
bool operator& (const PathLiteral::Flag lhs, const PathLiteral::Flag rhs);


/**
 * Class for scoped variable handling.
 */
class VariableHandler {
public:
	/** Possible options for variable checking. */
	enum Checking {
		CHECKING_OFF, /**< Do not check if referenced variables exist. */
		CHECKING_WARN, /**< Warn if referenced variables do not exist. */
		CHECKING_ERROR /**< Produce an error if a referenced variable does not exist. */
	};
private:
	/* last element is inner most scope */
	VariableScopes varScopes; /**< Variable scoped. The first is the outer most scope. */
	DynamicVariableSet dynVariables; /**< Dynamic variables not affected by variable checking. */
public:
	/**
	 * Default constructor.
	 */
	explicit VariableHandler() {}
	
	/**
	 * Constructor.
	 *
	 * @param[in] map - variable map for the first scope
	 */
	explicit VariableHandler(const VariableMap & map) {
		this->addScope(map);
	}
	
	/**
	 * Adds a variable to the list of dynamic variables.
	 *
	 * @param[in] name - name of the dynamic variable to add
	 */
	void addDynamicVariable(const std::string & name) {
		this->dynVariables.insert(name);
	}
	
	/**
	 * Replaces the current list of dynamic variables with the given one.
	 *
	 * @param[in] vars - new list of dynamic variables
	 */
	void setDynamicVariables(const DynamicVariableSet & vars) {
		this->dynVariables = vars;
	}
	
	/**
	 * Returns a reference to the current list of dynamic variables.
	 *
	 * @return reference to dynamic variable list
	 */
	const DynamicVariableSet & getDynamicVariables() const {
		return this->dynVariables;
	}
	
	/**
	 * Removes a dynamic variable form the internal list.
	 *
	 * @param[in] name - name of the dynamic variable for removal
	 */
	void removeDynamicVariable(const std::string & name) {
		this->dynVariables.erase(name);
	}
	
	/**
	 * Removes all dynamic variables for the internal list.
	 */
	void clearDynamicVariables() {
		this->dynVariables.clear();
	}
	
	/**
	 * Adds a new empty variable scope (more local than the previous one).
	 *
	 * @return a reference to the current variable scope
	 */
	VariableMap & addScope() {
		this->varScopes.push_back(VariableMap());
		return this->varScopes.back();
	}
	
	/**
	 * Adds a new variable scope (more local than the previous one).
	 *
	 * @param[in] map - variable map for the new scope
	 * @return a reference to the current variable scope
	 */
	VariableMap & addScope(const VariableMap & map) {
		this->varScopes.push_back(map);
		return this->varScopes.back();
	}
	
	/**
	 * Returns a reference to all variable scopes.
	 * 
	 * @return reference to all variable scopes
	 */
	VariableScopes & getScopes() {
		return this->varScopes;
	}
	
	/**
	 * Returns a optional reference to the current variable scope or nothing if there is no current
	 * variable scope.
	 *
	 * @return current variable scope, if any
	 */
	boost::optional<VariableMap &> getCurrentScope() {
		if ( this->varScopes.empty() ) return boost::optional<VariableMap &>();
		return boost::optional<VariableMap &>(this->varScopes.back());
	}
	
	/**
	 * Removes the current variable scope, if there is any.
	 *
	 * @return true if there was a variable scope, else false
	 */
	bool removeScope() {
		if ( this->varScopes.empty() ) return false;
		this->varScopes.pop_back();
		return true;
	}
	
	/**
	 * Removes all variable scopes with all associated variables.
	 */
	void clearScopes() {
		this->varScopes.clear();
	}
	
	/**
	 * Returns the string literal associated to the given named variable, if there is such a variable.
	 *
	 * @param[in] name - name of the variable
	 * @return string literal associated to the given variable name, if any
	 */
	boost::optional<StringLiteral &> get(const std::string & name) {
		if ( this->varScopes.empty() ) return boost::optional<StringLiteral &>();
		BOOST_REVERSE_FOREACH(VariableMap & map, this->varScopes) {
			VariableMap::iterator element = map.find(name);
			if (element != map.end()) {
				return boost::optional<StringLiteral &>(element->second);
			}
		}
		return boost::optional<StringLiteral &>();
	}
	
	/**
	 * Returns the string literal associated to the given named variable, if there is such a variable.
	 *
	 * @param[in] name - name of the variable
	 * @return string literal associated to the given variable name, if any
	 */
	boost::optional<const StringLiteral &> get(const std::string & name) const {
		if ( this->varScopes.empty() ) return boost::optional<const StringLiteral &>();
		BOOST_REVERSE_FOREACH(const VariableMap & map, this->varScopes) {
			VariableMap::const_iterator element = map.find(name);
			if (element != map.end()) {
				return boost::optional<const StringLiteral &>(element->second);
			}
		}
		return boost::optional<const StringLiteral &>();
	}
	
	/**
	 * Sets a new variable or replaces the content of an existing one with the given string literal.
	 * References to other variables are replaced within the given string literal.
	 *
	 * @param[in] name - name of the variable
	 * @param[in] var - string literal to set
	 * @param[in] variableChecking - variable checking option
	 * @return reference to the string literal of the newly set variable
	 * @see Checking
	 * @throws pcf::exception::OutOfRange if there is no current variable scope
	 * @throws pcf::exception::SymbolUnknown if variable checking was set to error and a referenced variable is not available
	 */
	StringLiteral & set(const std::string & name, StringLiteral var, const Checking variableChecking = CHECKING_WARN) {
		if ( this->varScopes.empty() ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(var.getLineInfo()) + ": No scope for the given variable allocated.")
			);
		}
		std::string unknownVariableName;
		if ( ! var.replaceVariables(unknownVariableName, *this) ) {
			switch ( variableChecking ) {
			case CHECKING_ERROR:
				BOOST_THROW_EXCEPTION(
					pcf::exception::SymbolUnknown()
					<< pcf::exception::tag::Message(boost::lexical_cast<std::string>(var.getLineInfo()) + ": Trying to access unknown variable '" + unknownVariableName + "'.")
				);
				break;
			case CHECKING_WARN:
				std::cerr << var.getLineInfo() << ": Trying to access unknown variable '" << unknownVariableName << "'." << std::endl;
				break;
			default:
				/* do nothing */
				break;
			}
		}
		this->varScopes.back()[name] = var;
		return this->varScopes.back()[name];
	}
	
	/**
	 * Removes a given variable from all scopes.
	 *
	 * @param[in] name - name of the variable for removal
	 * @return true if a variable was removed, else false
	 */
	bool unset(const std::string & name) {
		if ( this->varScopes.empty() ) return false;
		bool result = false;
		BOOST_REVERSE_FOREACH(VariableMap & map, this->varScopes) {
			if (map.erase(name) > 0) result = true;
		}
		return result;
	}
	
	/**
	 * Removes a given variable from the current scope.
	 * 
	 * @param[in] name - name of the variable for removal
	 * @return true if a variable was removed, else false
	 */
	bool unsetCurrent(const std::string & name) {
		if ( this->varScopes.empty() ) return false;
		return (this->varScopes.back().erase(name) > 0);
	}
};


} /* namespace pp */


#endif /* __PP_VARIABLE_HPP__ */
