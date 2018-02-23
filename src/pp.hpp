/**
 * @file pp.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2018 Daniel Starke
 * @date 2015-01-24
 * @version 2016-12-30
 */
#ifndef __PP_HPP__
#define __PP_HPP__


#include <cctype>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/serialization/strong_typedef.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <pcf/os/Target.hpp>
#include <pcf/path/Utility.hpp>


using namespace std;
using namespace boost;
using namespace pcf::path;
using boost::optional;
namespace fs = ::boost::filesystem;
namespace po = ::boost::program_options;


/* needed for template specialization of validate() */
BOOST_STRONG_TYPEDEF(size_t, JobsArg)


/**
 * Called by boost::program_options to parse a JobsArg argument.
 *
 * @param[in,out] v - variable storage
 * @param[in] values - input argument vector
 * @param[in] - workaround parameter for partial template specialization
 * @param[in] - workaround parameter for partial template specialization
 * @tparam CharT - character type
 * @see http://www.boost.org/doc/libs/1_54_0/doc/html/program_options/howto.html#idp123425568
 */
template <typename CharT>
void validate(boost::any & v, const vector< basic_string<CharT> > & values, JobsArg *, int) {
	po::validators::check_first_occurrence(v);
	if (values.size() != 1) throw po::validation_error(po::validation_error::invalid_option_value);
	bool inPercent(false), hasDecimalComma(false);
	basic_string<CharT> valToParse;
	BOOST_FOREACH(const CharT c, values[0]) {
		if ( ::isdigit(static_cast<int>(c)) ) {
			if ( inPercent ) throw po::validation_error(po::validation_error::invalid_option_value);
			valToParse.push_back(c);
		} else if (c == static_cast<CharT>('%')) {
			inPercent = true;
		} else if (c == static_cast<CharT>('.')) {
			if ( hasDecimalComma ) throw po::validation_error(po::validation_error::invalid_option_value);
			hasDecimalComma = true;
			valToParse.push_back(c);
		} else if ( ! ::isspace(c) ) {
			throw po::validation_error(po::validation_error::invalid_option_value);
		}
	}
	try {
		if ( inPercent ) {
			const long long jobs = boost::math::llround(static_cast<float>(boost::thread::hardware_concurrency()) * boost::lexical_cast<float>(valToParse) / 100.0f);
			if (jobs < 0) throw po::validation_error(po::validation_error::invalid_option_value);
			v = boost::any(static_cast<JobsArg>(static_cast<size_t>(jobs < 1 ? 1 : jobs)));
		} else {
			const size_t jobs = boost::lexical_cast<size_t>(valToParse);
			v = boost::any(static_cast<JobsArg>(jobs < 1 ? boost::thread::hardware_concurrency() : jobs));
		}
	} catch (...) {
		throw po::validation_error(po::validation_error::invalid_option_value);
	}
}


void terminate(boost::asio::io_service & ios, boost::scoped_ptr<boost::asio::io_service::work> & wn, volatile bool & stopped, const bool outputInfo);
void printHelp();
void printLicense();


#endif /* __PP_HPP__ */
