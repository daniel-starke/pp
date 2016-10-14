/**
 * @file Utility.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-08-13
 * @version 2016-05-01
 */
#include <cstring>
#include <new>
#include <vector>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/process/Utility.hpp>


namespace pcf {
namespace process {


namespace detail {


/**
 * Standard escaped character replacements (as in C).
 */
struct EscapedChar : boost::spirit::qi::symbols<char, char> {
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
} escapedChar;


/**
 * Splits the given string into an argument vector using shell syntax.
 *
 * @param[in] str - input command-line string to split
 * @param[in] sig - type signature
 * @param[out] vec - output argument vector
 * @internal
 */
template <typename CharT>
static void split(const std::basic_string<CharT> & cmdLine, const ProcessPipeFactory::Type::ShellTag &, std::vector< std::basic_string<CharT> > & outVec) {
	typedef typename std::basic_string<CharT>::const_iterator IteratorType;
	typedef std::vector< typename std::basic_string<CharT> > ArgumentListType;
	namespace phx = boost::phoenix;
	namespace qi = boost::spirit::qi;
	namespace encoding = boost::spirit::standard;
	using encoding::char_;
	using phx::push_back;
	using qi::_r1;
	using qi::_1;
	using qi::_val;
	using qi::blank;
	using qi::lit;
	qi::uint_parser<CharT, 16, 2, 2> hexOctet;
	
	/* syntax */
	qi::rule<IteratorType> cmdDelim = (
		blank
	);
	
	qi::rule<IteratorType, CharT()> basicChar = (
		(lit('\\') >> ((lit('x') >> hexOctet) | detail::escapedChar | char_)) | char_
	);
	
	qi::rule<IteratorType, std::basic_string<CharT>()> argument = (
		*(!cmdDelim >> basicChar)
	);
	
	qi::rule<IteratorType, std::basic_string<CharT>(CharT)> quotedArgument = (
		lit(_r1) >> *(!lit(_r1) >> basicChar) >> lit(_r1)
	);
	
	qi::rule<IteratorType, ArgumentListType()> argumentList = (
		(quotedArgument('"') | quotedArgument('\'') | argument)[push_back(_val, _1)] % (+cmdDelim)
	);
	
	/* parser */
	IteratorType first(cmdLine.begin()), last(cmdLine.end());
	const bool result = qi::parse(first, last, argumentList, outVec);
	if (first != last || result == false) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::SyntaxError()
			<< pcf::exception::tag::Message("Invalid command-line string.")
		);
	}
}


/**
 * Helper function to return the absolute path for the given binary.
 *
 * @param[in] path - path to the binary to resolve
 * @return resolved binary path
 * @remarks This is a template function to let the compiler decide which implementation is needed.
 */
template <typename CharT>
std::basic_string<CharT> resolveExecutable(const std::basic_string<CharT> & path);


template<>
std::basic_string<char> resolveExecutable(const std::basic_string<char> & path) {
	return pcf::path::resolveExecutable(boost::filesystem::path(path, pcf::path::utf8)).string(pcf::path::utf8);
}


template<>
std::basic_string<wchar_t> resolveExecutable(const std::basic_string<wchar_t> & path) {
	return pcf::path::resolveExecutable(boost::filesystem::path(path, pcf::path::utf8)).wstring(pcf::path::utf8);
}


} /* namespace detail */


/**
 * Helper function to combine multiple FDIO modes.
 * 
 * @see tFdioPMode
 */
tFdioPMode operator |(const tFdioPMode a, const tFdioPMode b) {
	return static_cast<tFdioPMode>(static_cast<int>(a) | static_cast<int>(b));
}


const ProcessPipeFactory::Type::ShellTag ProcessPipeFactory::Type::Shell = {};
const ProcessPipeFactory::Type::RawTag ProcessPipeFactory::Type::Raw = {};


ProcessPipeFactory::ProcessPipeFactory(const string_type & binPath, const std::vector<string_type> & procArgs):
	binaryPath(detail::resolveExecutable(binPath))
{
	this->raw = false;
	this->initFromVector(procArgs);
}


ProcessPipeFactory::ProcessPipeFactory(const string_type & binPath, const string_type & cmdLine, const Type::ShellTag & sig):
	binaryPath(detail::resolveExecutable(binPath))
{
	std::vector<string_type> procArgs;
	ProcessPipeFactory::split(cmdLine, sig, procArgs);
	this->raw = false;
	this->initFromVector(procArgs);
}


ProcessPipeFactory::ProcessPipeFactory(const string_type & binPath, const string_type & cmdLine, const Type::RawTag & /* sig */):
	binaryPath(detail::resolveExecutable(binPath))
{
	std::vector<string_type> procArgs;
	procArgs.push_back(cmdLine);
	this->raw = true;
	this->initFromVector(procArgs);
}


ProcessPipeFactory::ProcessPipeFactory(const ProcessPipeFactory & o):
	binaryPath(o.binaryPath),
	args(NULL),
	argsSize(o.argsSize),
	raw(o.raw)
{
	if (o.args == NULL) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::NullPointer()
			<< pcf::exception::tag::Message("Invalid argument pointer.")
		);
	}
	this->args = reinterpret_cast<char_type **>(malloc(this->argsSize));
	if (this->args == NULL) throw std::bad_alloc();
	char_type * outPtr;
	char_type * lastInPtr = *(o.args);
	memcpy(this->args, o.args, this->argsSize);
	outPtr = reinterpret_cast<char_type *>(this->args) + (lastInPtr - reinterpret_cast<char_type *>(o.args));
	for (
		char_type ** oArg = o.args, ** tArg = this->args;
		*oArg != NULL;
		lastInPtr = *oArg, oArg++, tArg++, outPtr += (*oArg - lastInPtr)
	) {
		*tArg = outPtr;
	}
}


template <>
void ProcessPipeFactory::split(const std::basic_string<char> & cmdLine, const ProcessPipeFactory::Type::ShellTag & sig, std::vector< std::basic_string<char> > & outVec) {
	detail::split(cmdLine, sig, outVec);
}


template <>
void ProcessPipeFactory::split(const std::basic_string<wchar_t> & cmdLine, const ProcessPipeFactory::Type::ShellTag & sig, std::vector< std::basic_string<wchar_t> > & outVec) {
	detail::split(cmdLine, sig, outVec);
}


void ProcessPipeFactory::initFromVector(const std::vector<string_type> & procArgs) {
	size_t totalStrLen = 0;
	char_type * outPtr;
	char_type ** argsPtr;
	BOOST_FOREACH(const string_type & aArg, procArgs) {
		totalStrLen += aArg.size() + 1;
	}
	this->argsSize = (sizeof(char_type *) * (procArgs.size() + 1)) + (sizeof(char_type) * totalStrLen);
	this->args = reinterpret_cast<char_type **>(malloc(this->argsSize));
	if (this->args == NULL) throw std::bad_alloc();
	outPtr = reinterpret_cast<char_type *>(&(this->args[procArgs.size() + 1]));
	argsPtr = this->args;
	BOOST_FOREACH(const string_type & aArg, procArgs) {
		const std::string abc(aArg.begin(), aArg.end());
		const size_t strLen(aArg.size());
		std::memcpy(outPtr, &(aArg[0]), strLen * sizeof(char_type));
		*argsPtr = outPtr;
		outPtr += strLen;
		*outPtr = 0;
		outPtr++;
		argsPtr++;
	}
	*argsPtr = NULL;
}


} /* namespace process */
} /* namespace pcf */
