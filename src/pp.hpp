/**
 * @file pp.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-01-24
 * @version 2016-05-01
 */
#ifndef __PP_HPP__
#define __PP_HPP__


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
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/path/Utility.hpp>


using namespace std;
using namespace boost;
using namespace pcf::path;
using boost::optional;
namespace fs = ::boost::filesystem;
namespace po = ::boost::program_options;


void terminate(boost::asio::io_service & ios, boost::scoped_ptr<boost::asio::io_service::work> & wn);
void printHelp();
void printLicense();


#endif /* __PP_HPP__ */
