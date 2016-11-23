/**
 * @file Shell.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-09-17
 * @version 2016-11-19
 */
#ifndef __PP_SHELL_HPP__
#define __PP_SHELL_HPP__


#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include "Variable.hpp"


namespace pp {


/**
 * Structure to handle various shells.
 */
struct Shell {
	/**
	 * Data encoding types.
	 */
	enum Encoding {
		UTF8,
		UTF16
	};
	
	/**
	 * Structure to hold a single replacement.
	 */
	struct Replacement {
		boost::wregex search; /**< Search this pattern. */
		std::wstring replace; /**< Replace found pattern with this string. */
	};
	
	boost::filesystem::path path; /**< Path to the shell binary. */
	StringLiteral cmdLine; /**< Command-line with {*} or {@*} for the parameters. */
	std::vector<Replacement> replacement; /**< List of replacement patterns. */
	Encoding outputEncoding; /**< Assume this encoding for the command output. */
	bool raw; /**< True if the command-line shall not be quoted like in Linux before passing it to the shell. */
	
	bool addReplacement(const std::string & pattern);
	std::string replace(const std::string & str) const;
};


} /* namespace pp */


#endif /* __PP_SHELL_HPP__ */
