/**
 * @file pp.cpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-01-24
 * @version 2016-11-19
 */
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/asio.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/bind.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/locale.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <boost/version.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/os/Signal.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/string/Escape.hpp>
#include "pp/Script.hpp"
#include "license.hpp"
#include "posix_main.hpp"
#include "pp.hpp"


extern "C" {
#include <sqlite3.h>
}


using namespace std;
using namespace boost;
using namespace pcf::os;
using namespace pcf::path;
using namespace pcf::string;
using boost::optional;
namespace fs = ::boost::filesystem;
namespace lc = ::boost::locale;
namespace po = ::boost::program_options;
namespace pt = ::boost::posix_time;


#ifndef PP_VERSION
#define PP_VERSION "<unknown>"
#endif /* PP_VERSION */


#ifndef PP_AUTHOR
#define PP_AUTHOR "Daniel Starke"
#endif /* PP_AUTHOR */


/** pp - client entry point */
int posix_main(int argc, POSIX_ARG_TYPE ** argv, POSIX_ARG_TYPE ** enpv) {
	try {
		/* process program arguments */
		po::options_description desc("pp");
		po::positional_options_description descRemain;
		bool build;
		size_t jobs;
		bool printOnly;
		string verbosity;
		fs::path changeDir;
		fs::path scriptFile;
		vector<string> targetList;
		pp::VariableMap envVarMap;
		vector<_U(string)> remainingOptions;
		boost::system::error_code ec;
		po::variables_map vm;

		for (POSIX_ARG_TYPE ** enp = enpv; *enp != NULL; enp++) {
#if defined(PCF_IS_WIN) && defined(_UNICODE)
			const string parameter = lc::conv::utf_to_utf<char>(*enp);
#else /* ASCII */
			const string parameter = string(*enp);
#endif /* UNICODE */
			const optional<pp::VariableMap::value_type> keyValue = pp::getKeyValuePair(parameter, fs::path("environment variable"));
			if ( keyValue ) {
				envVarMap.insert(*keyValue);
			}
		}

		desc.add_options()
			("build,b", po::value<bool>(&build)->default_value(false)->implicit_value(true)->zero_tokens(), "")
			("change-directory,C", po::_U(value)<fs::path>(&changeDir), "")
			("file,f", po::_U(value)<fs::path>(&scriptFile)->default_value("process.parallel"), "")
			("help,h", "")
			("jobs,j", po::value<size_t>(&jobs)->default_value(boost::thread::hardware_concurrency()), "")
			("license", "")
			("print-only,n", po::value<bool>(&printOnly)->default_value(false)->implicit_value(true)->zero_tokens(), "")
			("positional-parameter", po::_U(value)< vector<_U(string)> >()->multitoken()->zero_tokens(), "")
			("verbosity,v", po::value<string>(&verbosity)->default_value(string("WARN")), "")
		;
		descRemain.add("positional-parameter", -1);
		po::store(
			po::_U(command_line_parser)(argc, argv)
				.style(po::command_line_style::unix_style)
				.options(desc)
				.positional(descRemain)
				.run(),
			vm
		);
		
		/* print program options help */
		if ( vm.count("help") ) {
			printHelp();
			return EXIT_SUCCESS;
		}
		
		/* print program license */
		if ( vm.count("license") ) {
			printLicense();
			return EXIT_SUCCESS;
		}

		po::notify(vm);

		/* parse positional parameters */
		if ( ! vm["positional-parameter"].empty() ) {
			BOOST_FOREACH(const _U(string) & str, vm["positional-parameter"].as< vector<_U(string)> >()) {
#if defined(PCF_IS_WIN) && defined(_UNICODE)
				const string parameter = lc::conv::utf_to_utf<char>(str);
#else /* ASCII */
				const string parameter = str;
#endif /* UNICODE */
				const optional<pp::VariableMap::value_type> keyValue = pp::getKeyValuePair(parameter, fs::path("command-line variable"));
				if ( keyValue ) {
					/* is an environment variable; set value to key (might overwrite old value) */
					envVarMap[keyValue->first] = keyValue->second;
				} else {
					/* is a target */
					targetList.push_back(parameter);
				}
			}
		}
		if ( targetList.empty() ) {
			/* use default target */
			targetList.push_back("default");
		}

		/* change directory if set */
		if ( vm.count("change-directory") ) {
			boost::filesystem::current_path(changeDir, ec);
			if ( ec ) {
				cerr << "Error: Failed to change directory to \"" << changeDir.string(utf8) << "\"." << endl;
				return EXIT_FAILURE;
			}
		}
		
		/* add program specific environment variables */
		{
			const pt::ptime now(pt::second_clock::universal_time());
			pt::time_facet * timeFacet = new pt::time_facet("%H:%M:%S");
			pt::time_facet * dateFacet = new pt::time_facet("%Y-%m-%d");
			ostringstream sout;
			envVarMap[pp::Script::preDefPrefix + "PATH"] = pp::StringLiteral(getExecutablePath().string(utf8), pp::Script::preDefLocation, pp::StringLiteral::RAW);
			envVarMap[pp::Script::preDefPrefix + "VERSION"] = pp::StringLiteral(PP_VERSION, pp::Script::preDefLocation, pp::StringLiteral::RAW);
#if defined(PCF_IS_WIN)
			envVarMap[pp::Script::preDefPrefix + "OS"] = pp::StringLiteral("windows", pp::Script::preDefLocation, pp::StringLiteral::RAW);
#else /* PCF_IS_WIN */
			envVarMap[pp::Script::preDefPrefix + "OS"] = pp::StringLiteral("unix", pp::Script::preDefLocation, pp::StringLiteral::RAW);
#endif /* ! PCF_IS_WIN */
			sout.imbue(std::locale(cout.getloc(), timeFacet));
			sout << now;
			envVarMap[pp::Script::preDefPrefix + "TIME"] = pp::StringLiteral(sout.str(), pp::Script::preDefLocation, pp::StringLiteral::RAW);
			sout.str(string());
			sout.imbue(std::locale(cout.getloc(), dateFacet));
			sout << now;
			envVarMap[pp::Script::preDefPrefix + "DATE"] = pp::StringLiteral(sout.str(), pp::Script::preDefLocation, pp::StringLiteral::RAW);
			sout.str(string());
			sout << jobs;
			envVarMap[pp::Script::preDefPrefix + "THREADS"] = pp::StringLiteral(sout.str(), pp::Script::preDefLocation, pp::StringLiteral::RAW);
			envVarMap[pp::Script::preDefPrefix + "TARGETS"] = pp::StringLiteral(boost::algorithm::join(targetList, ","), pp::Script::preDefLocation, pp::StringLiteral::RAW);
		}

		/* execute script */
		pp::VariableHandler vars(envVarMap);
		/* set initial configuration */
		pp::Configuration config;
		config.build = build;
		config.printOnly = printOnly;
		config.environmentVariables = true;
		config.variableChecking = true;
		config.nestedVariables = true;
		config.fullRecursiveMatch = false;
		config.removeTemporaries = true;
		config.cleanUpIncompletes = true;
		config.removeRemains = true;
		if ( vm["verbosity"].defaulted() ) {
			config.lockedVerbosity = false;
		} else {
			config.lockedVerbosity = true;
		}
		config.verbosity = boost::lexical_cast<pp::Verbosity>(verbosity);
		config.shell = "default";
		
		pp::Script script(config, vars, jobs);
		/* parse script file (might throw an exception) */
		script.setProgressFormat("%dY-%dM-%dD %lH:%lM:%lS: %c / %t commands executed, %p%%, ETA %re\n"); /* default format string */
		script.read(scriptFile);
		config.verbosity = script.getVerbosity(); /* get updated verbosity level */
		const bool printOnlyFlag = config.printOnly;
		bool isFirst = true;
		volatile bool stopped = false;
		boost::asio::io_service ioService;
		boost::scoped_ptr<boost::asio::io_service::work> workNotifier;
		pcf::os::BackgroundSignalHandler signals(
			boost::bind(&terminate, boost::ref(ioService), boost::ref(workNotifier), boost::ref(stopped), config.verbosity >= pp::VERBOSITY_DEBUG),
			SIGINT,
			SIGTERM
		);
		signals.asyncWaitForSignal();
		
		/* process targets from command-line */
		BOOST_FOREACH(const string & target, targetList) {
			bool successfullyPrepared = true;
			
			/* prepare parallel tasks (might throw an exception) */
			if ( ! script.prepare(target) ) successfullyPrepared = false;
			
			/* initialize workers */
			ioService.reset();
			workNotifier.reset(new boost::asio::io_service::work(ioService));
			boost::thread_group workerThreads;
			if (( ! printOnlyFlag) || successfullyPrepared) {
				/* start execution jobs */
				if (config.verbosity >= pp::VERBOSITY_DEBUG) cerr << "pp: starting " << jobs << " worker threads" << endl;
				for (size_t job = 1; job <= jobs; job++) {
					workerThreads.create_thread(
						boost::bind(&boost::asio::io_service::run, &ioService)
					);
				}
				if (config.verbosity >= pp::VERBOSITY_DEBUG) cerr << "pp: finished starting worker threads" << endl;
			}
			
			/* perform parallel tasks */
			if ( successfullyPrepared ) script.execute(target, ioService);
			
			/* wait until workers are done */
			workNotifier.reset();
			if (( ! printOnlyFlag) || successfullyPrepared) {
				if (config.verbosity >= pp::VERBOSITY_DEBUG) cerr << "pp: waiting for worker threads to finish" << endl;
				workerThreads.join_all();
				if (config.verbosity >= pp::VERBOSITY_DEBUG) cerr << "pp: all workers finished" << endl;
				if ( successfullyPrepared ) {
					script.complete(target, isFirst);
				}
			}
			
			/* handle execution termination via signal */
			if ( stopped ) break;
		}
		
		signals.cancel();
	} catch (const pcf::exception::Script & e) {
		if (const string * errMsg = boost::get_error_info<pcf::exception::tag::Message>(e)) {
			cerr << *errMsg << endl;
		} else {
			cerr << "Error: Unknown syntax error." << endl;
		}
		return EXIT_FAILURE;
	} catch (const pcf::exception::Api & e) {
		if (const string * errMsg = boost::get_error_info<pcf::exception::tag::Message>(e)) {
			cerr << "Error: " << *errMsg << endl;
		} else {
			cerr << boost::diagnostic_information(e);
		}
		return EXIT_FAILURE;
	} catch (const po::invalid_command_line_syntax & e) {
		cerr << "Error: " << e.what() << endl;
		return EXIT_FAILURE;
	} catch (const po::multiple_occurrences & e) {
		cerr << "Error: " << e.what() << endl;
		return EXIT_FAILURE;
	} catch (const po::unknown_option & e) {
		cerr << "Error: " << e.what() << endl;
		return EXIT_FAILURE;
	} catch (const boost::exception & e) {
		cerr << boost::diagnostic_information(e);
		return EXIT_FAILURE;
	} catch (const std::exception & e) {
		cerr << "Exception: " << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


/**
 * Stops the process execution.
 *
 * @param[in,out] ios - I/O service to stop
 * @param[in,out] wn - worker notifier to clear
 * @param[out] stopped - set to true if execution was stopped
 * @param[in] outputInfo - set true to output info on termination to stderr
 */
void terminate(boost::asio::io_service & ios, boost::scoped_ptr<boost::asio::io_service::work> & wn, volatile bool & stopped, const bool outputInfo) {
	static volatile size_t num(0);
	if (num++ <= 0) {
		stopped = true;
		wn.reset();
		ios.stop();
		if ( outputInfo ) cerr << "pp: terminating execution" << endl;
	}
}


/**
 * Outputs the command line help.
 */
void printHelp() {
	cout <<
	"pp -bCf?hj?nv? [<target> ...] [<variable>=<value> ...]\n"
	"\n"
	" -b, --build\n"
	"  Forces all parts to be executed.\n"
	" -C, --change-directory <directory>\n"
	"  Change to this directory beforehand.\n"
	" -f, --file <file>\n"
	"  Defines the path to the script file.\n"
	"  The default value is \"process.parallel\".\n"
	" -h, --help\n"
	"  Prints out this description.\n"
	" -j, --jobs <number>\n"
	"  Execute with the given number of threads.\n"
	"  The default is the number of virtual cores.\n"
	" --license\n"
	"  Displays the licenses for this program.\n"
	" -n, --print-only\n"
	"  Only prints the commands that would had been executed.\n"
	" -v, --verbosity <enumeration>\n"
	"  Sets the verbosity level. Default is WARN.\n"
	"  Possible values are ERROR, WARN, INFO and DEBUG.\n"
	"\n"
	"target\n"
	"  Execute these targets in sequence.\n"
	"  The default target is \"default\".\n"
	"\n"
	"variable=value\n"
	"  Set these additional environment variables to the given value.\n"
	"\n"
	"Boost version: " << replace_all_copy(string(BOOST_LIB_VERSION), "_", ".") << "\n"
	"SQLite3 version: " SQLITE_VERSION "\n"
	"\n"
	"Parallel Processor " PP_VERSION "\n"
	"(c) Copyright 2015-2016 " PP_AUTHOR
	<< endl;
}


/**
 * Outputs the program license.
 */
void printLicense() {
	cout << licenseText << endl;
}
