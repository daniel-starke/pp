/**
 * @file Utility.cpp
 * @author Daniel Starke
 * @copyright Copyright 2013-2018 Daniel Starke
 * @date 2013-11-23
 * @version 2018-06-18
 */
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/locale.hpp>
#include <boost/regex.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/os/Target.hpp>
#include <pcf/path/Utility.hpp>
#include <pcf/string/Wildcard.hpp>


#if defined(PCF_IS_WIN)
extern "C" {
#include <cstdlib>
#include <windows.h>
}
#elif defined(PCF_IS_LINUX)
extern "C" {
#include <errno.h>
#include <unistd.h>
}
#endif /* PCF_IS_LINUX */


namespace pcf {
namespace path {


/** for path character encoding as UTF-8 */
boost::filesystem::detail::utf8_codecvt_facet utf8;


/**
 * The function modifies the path by removing all multiple separator
 * characters and turning \ to /.
 *
 * @param[in] p - path to correct
 * @return corrected path
 */
boost::filesystem::path correctSeparator(const boost::filesystem::path & p) {
	std::string str, newStr;
	std::string::value_type lastC;
	str = p.generic_string(utf8);
	newStr.reserve(str.size());
	lastC = 0;
	BOOST_FOREACH(std::string::value_type c, str) {
		switch (c) {
		case '\\':
		case '/':
			if (lastC != '/') {
				newStr.push_back('/');
				lastC = '/';
			}
			break;
		default:
			newStr.push_back(c);
			lastC = c;
			break;
		}
	}
	return boost::filesystem::path(newStr, utf8);
}


/**
 * The function normalizes the passed path. Symbolic links are ignored.
 * This function is mainly used to normalize virtual paths.
 * Use boost::filesystem::canonical for real paths.
 *
 * @param[in] p - path to normalize
 * @param[in] correctSep - set true to correct the separator character as well (default: false)
 * @return normalized path
 * @throws pcf::exception::OutOfRange if breaking the top level
 */
boost::filesystem::path normalize(const boost::filesystem::path & p, const bool correctSep) {
	boost::filesystem::path result;
	boost::filesystem::path::const_iterator i = p.begin();
	for (; i != p.end(); ++i) {
		if (*i == ".") continue;
		else if (*i == "..") {
			if ( result.empty() ) {
				/* got ".." but no more higher levels are available */
				BOOST_THROW_EXCEPTION(
					pcf::exception::OutOfRange()
					<< pcf::exception::tag::Message("Number of \"..\" entries is too high.")
				);
				return boost::filesystem::path();
			} else {
				result = result.parent_path();
			}
		} else {
			result /= *i;
		}
	}
	if ( correctSep ) {
		return correctSeparator(result);
	}
	return result;
}


/**
 * The functions returns the canonical path by solving the existing
 * part of the path for symbolic links and relative path parts.
 *
 * @param[in] p - path to canonicalize
 * @param[in] correctSep - set true to correct the separator character as well (default: false)
 * @return canonicalized path
 * @throws pcf::exception::OutOfRange if breaking the top level
 */
boost::filesystem::path original(const boost::filesystem::path & p, const bool correctSep) {
	boost::filesystem::path absPath = boost::filesystem::absolute(p);
	boost::filesystem::path::const_iterator i = absPath.begin();
	boost::filesystem::path result = *i++;
	for (; boost::filesystem::exists(result / *i) && i != absPath.end(); ++i) {
		result /= *i;
	}
	result = boost::filesystem::canonical(result);
	for (; i != absPath.end(); ++i) {
		if (*i == ".") continue;
		else if (*i == "..") {
			if ( result.empty() ) {
				/* got ".." but no more higher levels are available */
				BOOST_THROW_EXCEPTION(
					pcf::exception::OutOfRange()
					<< pcf::exception::tag::Message("Number of \"..\" entries is too high.")
				);
				return boost::filesystem::path();
			} else {
				result = result.parent_path();
			}
		}
		result /= *i;
	}
	if ( correctSep ) {
		return correctSeparator(result);
	}
	return result;
}


/**
 * The function checks if the passed path starts with the
 * given base path.
 * 
 * @param[in] base - base path
 * @param[in] path - path to check against
 * @return true, if path starts with base, else false
 */
bool hasBase(const boost::filesystem::path & base, const boost::filesystem::path & path) {
	const boost::filesystem::path a = normalize(base);
	const boost::filesystem::path b = normalize(path);
	boost::filesystem::path::const_iterator i, k;
	for (i = a.begin(), k = b.begin(); i != a.end() && k != b.end(); ++i, k++) {
		if (*i != *k) break;
	}
	return (i == a.end());
}


/**
 * The function removes the given base path from the passed path to make
 * it relative from there on.
 *
 * @param[in] base - base path
 * @param[in] path - path to make relative from base
 * @param[in] relative - true to make removed part relative, else false
 * @return new path or old path if path did not start with base
 */
boost::filesystem::path removeBase(const boost::filesystem::path & base, const boost::filesystem::path & path, const bool relative) {
	boost::filesystem::path::const_iterator b, bEnd = base.end(), p, pEnd = path.end();
	for (b = base.begin(), p = path.begin(); b != bEnd && p != pEnd && (*b) == (*p); ++b, ++p);
	if (b != bEnd) {
		return path;
	} else {
		boost::filesystem::path result;
		bool isFirst = true;
		if ( relative ) {
			result = ".";
			isFirst = false;
		}
		for (; p != pEnd; ++p) {
			if ( isFirst ) {
				result = *p;
				isFirst = false;
			} else {
				result /= *p;
			}
		}
		return result;
	}
}


/**
 * Tries to resolve the path to the given executable in the same way the system
 * command-line shell would do.
 *
 * @param[in] path - path to resolve
 * @return resolved absolute executable path or an empty path on error
 * @throws error code on error
 * @see exec() for Linux
 * @see CreateProcess() for Windows
 */
boost::filesystem::path resolveExecutable(const boost::filesystem::path & path) {
	boost::system::error_code errorCode;
	const boost::filesystem::path result = resolveExecutable(path, errorCode);
	if ( errorCode ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::SystemError()
			<< pcf::exception::tag::Message(path.string(utf8) + ": " + errorCode.message())
			<< pcf::exception::tag::ErrorCode(errorCode.value())
		);
	}
	return result;
}


/**
 * Tries to resolve the path to the given executable in the same way the system
 * command-line shell would do.
 *
 * @param[in] path - path to resolve
 * @param[out] errorCode - returns the result error code
 * @return resolved absolute executable path or an empty path on error
 * @see exec() for Linux
 * @see CreateProcess() for Windows
 */
boost::filesystem::path resolveExecutable(const boost::filesystem::path & path, boost::system::error_code & errorCode) {
	boost::filesystem::path result;
	errorCode = boost::system::errc::make_error_code(boost::system::errc::success);
	boost::filesystem::path lastPath;
	boost::system::error_code ec;
#if defined(PCF_IS_WIN)
	std::vector<boost::filesystem::path> searchPaths, extPaths;
	/* get paths */
	std::wstring extension(path.extension().wstring(utf8));
	std::wstring splitPart;
	const size_t envPathExtLen = GetEnvironmentVariableW(L"PATHEXT", NULL, 0);
	std::wstring envPathExt(envPathExtLen, 0);
	envPathExt.resize(GetEnvironmentVariableW(L"PATHEXT", &(envPathExt[0]), static_cast<DWORD>(envPathExtLen)));
	bool tryAllExtensions = true;
	boost::algorithm::to_lower(extension);
	boost::algorithm::to_lower(envPathExt);
	splitPart.reserve(envPathExt.size());
	/* add possible file extensions to list of file extensions */
	BOOST_FOREACH(const wchar_t c, envPathExt) {
		if (c == L';') {
			if ( ! splitPart.empty() ) {
				if (splitPart == extension) {
					tryAllExtensions = false;
					break;
				}
				extPaths.push_back(boost::filesystem::path(splitPart, utf8));
				splitPart.clear();
			}
		} else if (c != 0) {
			splitPart.push_back(c);
		}
	}
	if (tryAllExtensions && ( ! splitPart.empty() )) extPaths.push_back(boost::filesystem::path(splitPart, utf8));
	/* check if absolute path was given */
	if ( path.has_root_directory() ) {
		/* absolute path */
		if ( ! tryAllExtensions ) {
			/* check full path */
			if ( boost::filesystem::is_regular_file(path, ec) ) {
				if ( ! Permission(path).executable() ) {
					/* permission for execution not given */
					errorCode = boost::system::errc::make_error_code(boost::system::errc::permission_denied);
				}
				/* found executable file */
				return path;
			} else {
				errorCode = ec;
				return boost::filesystem::path();
			}
		}
		/* try different file extensions */
		BOOST_FOREACH(const boost::filesystem::path & ext, extPaths) {
			const boost::filesystem::path fullPath(boost::filesystem::path(path).concat(ext.native(), utf8));
			/* check full path */
			if ( boost::filesystem::is_regular_file(fullPath, ec) ) {
				if ( ! Permission(fullPath).executable() ) {
					/* permission for execution not given */
					lastPath = fullPath;
					continue;
				}
				/* found executable file */
				errorCode = boost::system::errc::make_error_code(boost::system::errc::success);
				return boost::filesystem::system_complete(fullPath, ec);
			} else {
				errorCode = ec;
			}
		}
		if ( lastPath.empty() ) {
			if ( ! errorCode ) {
				errorCode = boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory);
			}
			return boost::filesystem::path();
		} else {
			/* permission for execution not given */
			errorCode = boost::system::errc::make_error_code(boost::system::errc::permission_denied);
			return boost::filesystem::system_complete(lastPath, ec);
		}
	}
	const size_t curPathLen = GetCurrentDirectoryW(0, NULL);
	const size_t sysPathLen = GetSystemDirectoryW(NULL, 0);
	const size_t envPathLen = GetEnvironmentVariableW(L"PATH", NULL, 0);
	std::wstring curPath(curPathLen, 0);
	std::wstring sysPath(sysPathLen, 0);
	std::wstring envPath(envPathLen, 0);
	std::wstring extionsion(path.extension().wstring());
	boost::algorithm::to_lower(extension);
	curPath.resize(GetCurrentDirectoryW(static_cast<DWORD>(curPathLen), &(curPath[0])));
	sysPath.resize(GetSystemDirectoryW(&(sysPath[0]), static_cast<UINT>(sysPathLen)));
	envPath.resize(GetEnvironmentVariableW(L"PATH", &(envPath[0]), static_cast<DWORD>(envPathLen)));
	/* add possible paths to list of search paths */
	searchPaths.push_back(getExecutablePath(ec).parent_path());
	if ( ec ) {
		errorCode = ec;
		return boost::filesystem::path();
	}
	searchPaths.push_back(boost::filesystem::path(curPath, utf8));
	searchPaths.push_back(boost::filesystem::path(sysPath, utf8));
	splitPart.clear();
	BOOST_FOREACH(const wchar_t c, envPath) {
		if (c == L';') {
			if ( ! splitPart.empty() ) {
				searchPaths.push_back(boost::filesystem::path(splitPart, utf8));
				splitPart.clear();
			}
		} else if (c != 0) {
			splitPart.push_back(c);
		}
	}
	if ( ! splitPart.empty() ) searchPaths.push_back(boost::filesystem::path(splitPart, utf8));
	/* search all paths until executable file is found or end of list has been reached */
	BOOST_FOREACH(const boost::filesystem::path & p, searchPaths) {
		const boost::filesystem::path absolutePath(boost::filesystem::path(p) / path);
		if ( ! tryAllExtensions ) {
			/* check full path */
			if ( boost::filesystem::is_regular_file(absolutePath, ec) ) {
				if ( ! Permission(absolutePath).executable() ) {
					/* permission for execution not given */
					errorCode = boost::system::errc::make_error_code(boost::system::errc::permission_denied);
					lastPath = absolutePath;
					continue;
				}
				/* found executable file */
				errorCode = boost::system::errc::make_error_code(boost::system::errc::success);
				return absolutePath;
			}
		} else {
			/* try different file extensions */
			BOOST_FOREACH(const boost::filesystem::path & ext, extPaths) {
				const boost::filesystem::path fullPath(boost::filesystem::path(absolutePath).concat(ext.native(), utf8));
				/* check full path */
				if ( boost::filesystem::is_regular_file(fullPath, ec) ) {
					if ( ! Permission(fullPath).executable() ) {
						/* permission for execution not given */
						lastPath = fullPath;
						continue;
					}
					/* found executable file */
					errorCode = boost::system::errc::make_error_code(boost::system::errc::success);
					return boost::filesystem::system_complete(fullPath, ec);
				} else {
					errorCode = ec;
				}
			}
		}
	}
	if ( lastPath.empty() ) {
		if ( ! errorCode ) {
			errorCode = boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory);
		}
		return boost::filesystem::path();
	} else {
		/* permission for execution not given */
		errorCode = boost::system::errc::make_error_code(boost::system::errc::permission_denied);
		return boost::filesystem::system_complete(lastPath, ec);
	}
#else /* ! PCF_IS_WIN */
	/* check if absolute path was given */
	if ( path.has_root_directory() ) {
		/* check full path */
		if ( boost::filesystem::is_regular_file(path, ec) ) {
			if ( ! Permission(path).executable() ) {
				/* permission for execution not given */
				errorCode = boost::system::errc::make_error_code(boost::system::errc::permission_denied);
			} else {
				/* found executable file */
				return path;
			}
			result = path.filename();
		} else {
			errorCode = ec;
			return boost::filesystem::path();
		}
	} else {
		result = path;
	}
	/* relative path was given */
	std::vector<boost::filesystem::path> searchPaths;
	const char * envPathStr = getenv("PATH");
	/* add possible paths to list of search paths */
	if (envPathStr != NULL) {
		const std::string envPath(envPathStr);
		std::string splitPart;
		BOOST_FOREACH(const char c, envPath) {
			if (c == ';') {
				if ( ! splitPart.empty() ) {
					searchPaths.push_back(boost::filesystem::path(splitPart, utf8));
					splitPart.clear();
				}
			} else if (c != 0) {
				splitPart.push_back(c);
			}
		}
	}
	{
		const boost::filesystem::path curPath(boost::filesystem::current_path(ec));
		if ( ! ec ) {
			searchPaths.push_back(curPath);
		}
	}
#if _POSIX_C_SOURCE >= 2 || defined(_XOPEN_SOURCE)
	{
		size_t envPathLen(confstr(_CS_PATH, NULL, 0));
		if (envPathLen > 0) {
			std::string envPath(envPathLen, 0);
			while ((envPathLen = confstr(_CS_PATH, &(envPath[0]), envPathLen)) != envPath.size()) {
				envPath.resize(envPathLen);
			}
			envPath.resize(envPathLen - 1); /* remove null-termination */
			if ( ! envPath.empty() ) {
				std::string splitPart;
				BOOST_FOREACH(const char c, envPath) {
					if (c == ';') {
						if ( ! splitPart.empty() ) {
							searchPaths.push_back(boost::filesystem::path(splitPart, utf8));
							splitPart.clear();
						}
					} else if (c != 0) {
						splitPart.push_back(c);
					}
				}
			}
		}
	}
#endif /* confstr */
	/* search all paths until executable file is found or end of list has been reached */
	lastPath = result;
	BOOST_FOREACH(const boost::filesystem::path & p, searchPaths) {
		const boost::filesystem::path absolutePath(boost::filesystem::path(p) / result);
		/* check full path */
		if ( boost::filesystem::is_regular_file(absolutePath, ec) ) {
			if ( ! Permission(absolutePath).executable() ) {
				/* permission for execution not given */
				errorCode = boost::system::errc::make_error_code(boost::system::errc::permission_denied);
			} else {
				/* found executable file */
				errorCode = boost::system::errc::make_error_code(boost::system::errc::success);
				return absolutePath;
			}
			lastPath = absolutePath;
		}
	}
	if ( lastPath.empty() ) {
		if ( ! errorCode ) {
			errorCode = boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory);
		}
		return boost::filesystem::path();
	} else {
		/* permission for execution not given */
		errorCode = boost::system::errc::make_error_code(boost::system::errc::permission_denied);
		return boost::filesystem::system_complete(lastPath, ec);
	}
#endif /* ! PCF_IS_WIN */
	return result;
}


/**
 * Returns the path to the running executable.
 *
 * @return path to own executable
 * @throws error code on error
 */
boost::filesystem::path getExecutablePath() {
	boost::system::error_code errorCode;
	const boost::filesystem::path result = getExecutablePath(errorCode);
	if ( errorCode ) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::SystemError()
			<< pcf::exception::tag::Message(errorCode.message())
			<< pcf::exception::tag::ErrorCode(errorCode.value())
		);
	}
	return result;
}


/**
 * Returns the path to the running executable.
 *
 * @param[out] errorCode - returns the result error code
 * @return path to own executable
 */
boost::filesystem::path getExecutablePath(boost::system::error_code & errorCode) {
	errorCode = boost::system::errc::make_error_code(boost::system::errc::success);
#if defined(PCF_IS_WIN)
	DWORD result = 0;
	std::wstring exePath;
	size_t exePathLen = 0;
	for (
		result = static_cast<DWORD>(exePathLen = MAX_PATH + 1);
		static_cast<size_t>(result) >= exePathLen || result == ERROR_INSUFFICIENT_BUFFER;
		exePathLen += MAX_PATH
	) {
		exePath.resize(exePathLen);
		result = GetModuleFileNameW(NULL, &(exePath[0]), static_cast<DWORD>(exePathLen));
	}
#else /* ! PCF_IS_WIN */
	ssize_t result = 0;
	std::string exePath;
	size_t exePathLen = 0;
	const char * const exeLink = "/proc/self/exe";
	for (
		result = static_cast<ssize_t>(exePathLen = PATH_MAX);
		static_cast<size_t>(result) >= exePathLen;
		exePathLen += PATH_MAX
	) {
		exePath.resize(exePathLen);
		result = readlink(exeLink, &(exePath[0]), exePathLen);
		if (result < 0) {
			errorCode = boost::system::error_code(errno, boost::system::generic_category());
			return boost::filesystem::path();
		}
#if defined(__CYGWIN__)
		exePath.resize(static_cast<size_t>(result));
		if ( boost::filesystem::is_regular_file(boost::filesystem::path(exePath, utf8)) ) {
			return boost::filesystem::path(exePath, utf8);
		} else if ( boost::filesystem::is_regular_file(boost::filesystem::path(exePath + ".exe", utf8)) ) {
			return boost::filesystem::path(exePath + ".exe", utf8);
		} else {
			errorCode = boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory);
			return boost::filesystem::path();
		}
#endif
	}
#endif /* ! PCF_IS_WIN */
	exePath.resize(static_cast<size_t>(result));
	return boost::filesystem::path(exePath, utf8);
}


/**
 * Adds a list of matching paths to the given output vector.
 * Elements that are already in the output vector are preserved.
 *
 * @param[in,out] r - output path vector
 * @param[in] b - already matched base part of the pattern path matching expression (p)
 * @param[in] p - path matching expression
 * @param[in] matchAll - set to true to check against all files recursively, false to match only path elements
 * @param[in] hasPattern - callback function to check if a path element contains a pattern
 * @param[in] unescapePattern - callback function to unescape a path element which contains a pattern (but only for literals)
 * @param[in] matchesPattern - callback function to check if a path element matches a pattern
 * @return true on success, else false
 * @see CheckPatternCallback and MatchingCallback for callback definitions
 */
bool getMatchingPathList(std::vector<boost::filesystem::path> & r, const boost::filesystem::path & b, const boost::filesystem::path & p, const bool matchAll, const CheckPatternCallback & hasPattern, const UnescapePatternCallback & unescapePattern, const MatchingCallback & matchesPattern) {
	boost::filesystem::path realPath, basePath;
	boost::system::error_code ec;
	boost::filesystem::path::const_iterator next, i = p.begin();
	boost::filesystem::path::const_iterator end = p.end();
	/* check pattern */
	if ( p.empty() ) return true;
	/* build real (existing) path */
	if (b.empty() || !boost::filesystem::exists(b, ec) || ec) {
		for(; i != end; ++i) {
			if ( hasPattern(i->wstring(utf8)) ) {
				/* end of non-pattern path elements */
				break;
			} else {
				if (boost::filesystem::exists(realPath / unescapePattern(i->wstring(utf8)), ec) && ( ! ec )) {
					realPath /= unescapePattern(i->wstring(utf8));
				} else {
					/* end of non-pattern and non-real path elements */
					break;
				}
			}
		}
		/* build base (partly existing) path */
		basePath = realPath;
		for(; i != end; ++i) {
			if ( hasPattern(i->wstring(utf8)) ) {
				/* end of non-pattern path elements */
				break;
			} else {
				basePath /= unescapePattern(i->wstring(utf8));
			}
		}
	} else {
		realPath = b;
		basePath = realPath;
		boost::filesystem::path::const_iterator k = basePath.begin();
		boost::filesystem::path::const_iterator kEnd = basePath.end();
		for (; i != end && k != kEnd && *i == *k; ++i, ++k);
	}
	/* passed path does not exists */
	if (realPath != basePath) return false;
	if (i == end) {
		r.push_back(realPath);
		return true;
	}
	/* build string match list */
	for(; i != end; i++) {
		const std::wstring pattern(i->wstring(utf8));
		next = i;
		next++;
		if ( hasPattern(pattern) ) {
			/* string matching */
			bool result = true;
			if ( matchAll ) {
				/* match whole part from here */
				const bool removeBasePath = basePath.empty();
				if ( removeBasePath ) basePath = boost::filesystem::current_path();
				for (boost::filesystem::recursive_directory_iterator n(basePath), endN; n != endN; ++n) {
					const std::wstring element(n->path().filename().wstring(utf8));
					const std::wstring relativePart(removeBase(basePath, n->path()).generic_wstring(utf8));
					const std::wstring relativePattern(buildPathFromIterator(i, end).generic_wstring(utf8));
					if (element == L"." || element == L"..") continue;
					if ( matchesPattern(relativePart, relativePattern) ) {
						if ( removeBasePath ) {
							r.push_back(relativePart);
						} else {
							r.push_back(realPath / relativePart);
						}
					}
				}
				break;
			} else {
				/* match element-wise from here */
				const bool removeBasePath = basePath.empty();
				if ( removeBasePath ) basePath = boost::filesystem::current_path();
				for (boost::filesystem::directory_iterator n(basePath), endN; n != endN; ++n) {
					const std::wstring element(n->path().filename().wstring(utf8));
					if (element == L"." || element == L"..") continue;
					if ( matchesPattern(element, pattern) ) {
						if (next == end) {
							if ( removeBasePath ) {
								r.push_back(removeBase(basePath, n->path()));
							} else {
								r.push_back(n->path());
							}
						} else if ( ! boost::filesystem::is_regular_file(n->status()) ) {
							if ( removeBasePath ) {
								result = result && getMatchingPathList(
									r,
									boost::filesystem::path(),
									buildPathFromIterator(removeBase(basePath, n->path()), next, end),
									false,
									hasPattern,
									unescapePattern,
									matchesPattern
								);
							} else {
								result = result && getMatchingPathList(
									r,
									n->path(),
									buildPathFromIterator(n->path(), next, end),
									false,
									hasPattern,
									unescapePattern,
									matchesPattern
								);
							}
						}
					}
				}
			}
			if ( ! result ) return false;
			break;
		} else {
			/* just copy path element */
			basePath /= unescapePattern(i->wstring(utf8));
			if (next == end) {
				/* last element */
				if (boost::filesystem::exists(basePath, ec) && ( ! ec )) {
					r.push_back(basePath);
				} else {
					return false;
				}
			}
		}
	}
	return true;
}

/**
 * Adds a list of matching paths to the given output vector.
 * Elements that are already in the output vector are preserved.
 *
 * @param[in,out] r - output path vector
 * @param[in] p - wildcard path matching expression
 * @param[in] matchAll - set to true to check against all files recursively, false to match only path elements
 * @return true on success, else false
 * @remarks Path matching is done case insensitive on Windows and case sensitive on any other platform.
 */
bool getWildcardPathList(std::vector<boost::filesystem::path> & r, const std::wstring & p, const bool matchAll) {
	/* callback function to decide whether a path element contains a pattern or not */
	struct CheckPatternCallbackNamespace {
		static bool isWildcardPattern(const std::wstring & str) {
			return (str.find_first_of(L"*?#") != std::wstring::npos);
		}
	};
	
	/* callback function to unescape a path element */
	struct UnescapeCallbackNamespace {
		static std::wstring unescapeWildcardPattern(const std::wstring & str) {
			return str;
		}
	};
	
	/* callback function to check whether the given path element matches the pattern or not */
	struct MatchingCallbackNamespace {
		static bool matchesWildcardPattern(const std::wstring & str, const std::wstring & pattern) {
#if defined(PCF_IS_WIN)
			return pcf::string::matchWildcard(str, pattern, pcf::string::wildcardOption::CASE_INSENSITIVE);
#else /* ! PCF_IS_WIN */
			return pcf::string::matchWildcard(str, pattern, pcf::string::wildcardOption::NONE);
#endif /* ! PCF_IS_WIN */
		}
	} ;

	return getMatchingPathList(
		r,
		boost::filesystem::path(),
		boost::filesystem::path(p, utf8),
		matchAll,
		CheckPatternCallbackNamespace::isWildcardPattern,
		UnescapeCallbackNamespace::unescapeWildcardPattern,
		MatchingCallbackNamespace::matchesWildcardPattern
	);
}


/**
 * Adds a list of matching paths to the given output vector.
 * Elements that are already in the output vector are preserved.
 *
 * @param[in,out] r - output path vector
 * @param[in] p - regular expression for path matching
 * @param[in] matchAll - set to true to check against all files recursively, false to match only path elements
 * @return true on success, else false
 * @remarks Path matching is done case insensitive on Windows and case sensitive on any other platform.
 * @remarks The path separator needs to be / instead of \ on all platforms. Use correctSeparator() if needed.
 * @throws pcf::exception::SyntaxError on regular expression syntax errors
 */
bool getRegexPathList(std::vector<boost::filesystem::path> & r, const std::wstring & p, const bool matchAll) {
	/* callback function to decide whether a path element contains a pattern or not */
	struct CheckPatternCallbackNamespace {
		static bool isRegexPattern(const std::wstring & str) {
			size_t pos = 0;
			while (pos != std::wstring::npos) {
				pos = str.find_first_of(L".[{()*+?|^$", pos);
				if (pos != std::wstring::npos) {
					if (pos > 0 && str[pos - 1] != 0x241B) {
						if (pos > 1 && (str[pos - 2] != 0x241B || str[pos] != L'{' || (str[pos - 1] != L'x' && str[pos - 1] != L'N'))) {
							return true;
						} else if (pos <= 1) {
							return true;
						}
					}
					pos++;
				}
			}
			return false;
		}
	};
	
	/* callback function to unescape a path element */
	struct UnescapeCallbackNamespace {
		static std::wstring unescapeRegexPattern(const std::wstring & str) {
			const boost::regex_traits_wrapper<boost::wregex::traits_type> traits;
			std::wstring result;
			std::wstring::value_type lastChar(0);
			result.reserve(str.size());
			std::wstring::const_pointer start(&(str[0])), end(&(str[str.size()]));
			for (; start != end; start++) {
				const std::wstring::value_type c = *start;
				if (lastChar == 0x241B) {
					switch (c) {
					case L'a': result.push_back(L'\a'); break;
					case L'e': result.push_back(0x001E); break;
					case L'f': result.push_back(L'\f'); break;
					case L'n': result.push_back(L'\n'); break;
					case L'r': result.push_back(L'\r'); break;
					case L't': result.push_back(L'\t'); break;
					case L'v': result.push_back(L'\v'); break;
					case L'c': result.push_back(c % 32); break;
					case L'x':
						{
							if (++start == end) goto onerror;
							if (*start == L'{') {
								if (++start == end) goto onerror;
								const int i = static_cast<int>(traits.toi(start, end, 16));
								if (start == end
									|| i < 0
									|| (std::numeric_limits<std::wstring::value_type>::is_specialized && i > static_cast<int>(std::numeric_limits<std::wstring::value_type>::max()))
									|| *start != L'}') {
									goto onerror;
								}
								result.push_back(static_cast<std::wstring::value_type>(i));
							} else {
								const std::ptrdiff_t len = std::min(static_cast<std::ptrdiff_t>(2), static_cast<std::ptrdiff_t>(end - start));
								const int i = static_cast<int>(traits.toi(start, start + len, 16));
								if (i < 0 || (std::numeric_limits<std::wstring::value_type>::is_specialized && i > static_cast<int>(std::numeric_limits<std::wstring::value_type>::max()))) {
									goto onerror;
								}
								start--;
								result.push_back(static_cast<std::wstring::value_type>(i));
							}
						}
						break;
					case L'0':
						{
							/* octal escape sequence */
							if (++start == end) goto onerror;
							const std::ptrdiff_t len = std::min(static_cast<std::ptrdiff_t>(3), static_cast<std::ptrdiff_t>(end - start));
							const int i = static_cast<int>(traits.toi(start, start + len, 8));
							if (i < 0 || (std::numeric_limits<std::wstring::value_type>::is_specialized && i > static_cast<int>(std::numeric_limits<std::wstring::value_type>::max()))) {
								goto onerror;
							}
							start--;
							result.push_back(static_cast<std::wstring::value_type>(i));
						}
						break;
					case L'N':
						{
							/* \N{name} named escape sequence */
							if (++start == end) goto onerror;
							if (*start != L'{') goto onerror;
							if (++start == end) goto onerror;
							const std::wstring::const_pointer newStart(std::find(start, end, L'}'));
							if (newStart == end) goto onerror;
							/* use boost's internal lookup table */
							const std::wstring res = traits.lookup_collatename(start, newStart);
							if ( res.empty() ) goto onerror;
							start = newStart;
							result.append(res);
						}
						break;
					case 0x241B: result.push_back(L'\\'); break;
					default: result.push_back(c); break;
					}
					lastChar = 0;
				} else if (c != 0x241B) {
					if (c == 0x241F) {
						result.push_back(L':');
					} else {
						result.push_back(c);
					}
					lastChar = c;
				} else {
					lastChar = c;
				}
				if (start == end) break;
			}
			return result;
			onerror:
			BOOST_THROW_EXCEPTION(
				pcf::exception::SyntaxError()
				<< pcf::exception::tag::Message("Invalid escape code in regular expression: " + boost::locale::conv::utf_to_utf<char>(str))
			);
			return std::wstring();
		}
	};
	
	/* callback function to check whether the given path element matches the pattern or not */
	struct MatchingCallbackNamespace {
		static bool matchesRegexPattern(const std::wstring & str, const std::wstring & pattern) {
			using namespace boost::regex_constants;
			std::wstring origPattern(pattern);
			/* revert escape character from Unicode to ASCII version */
			std::replace(origPattern.begin(), origPattern.end(), static_cast<std::wstring::value_type>(0x241B), L'\\');
			std::replace(origPattern.begin(), origPattern.end(), static_cast<std::wstring::value_type>(0x241F), L':');
			try {
				boost::wregex regex(
					origPattern,
					perl
#if defined(PCF_IS_WIN)
					| icase
#endif /* PCF_IS_WIN */
				);
				return boost::regex_match(str, regex);
			} catch (std::invalid_argument &) {
				BOOST_THROW_EXCEPTION(
					pcf::exception::SyntaxError()
					<< pcf::exception::tag::Message("Invalid regular expression: " + boost::locale::conv::utf_to_utf<char>(origPattern))
				);
			}
			return false;
		}
	} ;
	
	/* replace escape character to enable the boost::filesystem::path iterator for paths */
	std::wstring escapedPattern;
	std::wstring::value_type lastChar(0);
	size_t pos = 0;
	escapedPattern.reserve(p.size());
	BOOST_FOREACH(const std::wstring::value_type c, p) {
		pos++;
		if (lastChar == L'\\') {
			if (c == L'\\') {
				escapedPattern.push_back('/');
			} else {
				escapedPattern.push_back(0x241B);
				escapedPattern.push_back(c);
			}
			lastChar = 0;
			continue;
		} else if (c == L':') {
			/* replace with Unicode unit separator */
#if defined(PCF_IS_WIN)
			if (pos == 2) {
				escapedPattern.push_back(L':');
			} else
#endif /* PCF_IS_WIN */
			{
				escapedPattern.push_back(0x241F);
			}
		} else if (c != L'\\') {
			escapedPattern.push_back(c);
		}
		lastChar = c;
	}
	if (escapedPattern.size() > 1 && escapedPattern[1] == 0x241F) escapedPattern[1] = L':';

	return getMatchingPathList(
		r,
		boost::filesystem::path(),
		boost::filesystem::path(escapedPattern, utf8),
		matchAll,
		CheckPatternCallbackNamespace::isRegexPattern,
		UnescapeCallbackNamespace::unescapeRegexPattern,
		MatchingCallbackNamespace::matchesRegexPattern
	);
}



Permission::Permission(const boost::filesystem::path & p):
	path(p),
	pExists(false),
	pRead(false),
	pWrite(false),
	pExecute(false)
{
#if defined(PCF_IS_WIN)
	DWORD lpnLengthNeeded;
	SECURITY_DESCRIPTOR * security;
	/* get PSECURITY_DESCRIPTOR size */
	GetFileSecurityW(
		path.wstring().c_str(),
		OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
		static_cast<PSECURITY_DESCRIPTOR>(NULL),
		0,
		&lpnLengthNeeded
	);
	/* get access rights */
	security = reinterpret_cast<SECURITY_DESCRIPTOR *>(malloc(lpnLengthNeeded));
	if (security == reinterpret_cast<SECURITY_DESCRIPTOR *>(NULL)) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfMemory()
			<< pcf::exception::tag::Message("Could not allocate enough memory.")
		);
	}
	if (GetFileSecurityW(
		path.wstring().c_str(),
		OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
		static_cast<PSECURITY_DESCRIPTOR>(security),
		lpnLengthNeeded,
		&lpnLengthNeeded
	) != 0) {
		/* success */
		HANDLE hToken = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken)) {
			HANDLE hImpersonatedToken = NULL;
			if (DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken)) {
				DWORD genericAccessRights = MAXIMUM_ALLOWED;
				GENERIC_MAPPING mapping;
				PRIVILEGE_SET privileges;
				DWORD grantedAccess = 0;
				DWORD privilegesLength = static_cast<DWORD>(sizeof(privileges));
				BOOL result = FALSE;
				mapping.GenericRead = FILE_GENERIC_READ;
				mapping.GenericWrite = FILE_GENERIC_WRITE;
				mapping.GenericExecute = FILE_GENERIC_EXECUTE;
				mapping.GenericAll = FILE_ALL_ACCESS;
				MapGenericMask(&genericAccessRights, &mapping);
				if (AccessCheck(security, hImpersonatedToken, genericAccessRights,
					&mapping, &privileges, &privilegesLength, &grantedAccess, &result)) {
					this->pExists = true;
					this->pRead = (grantedAccess & FILE_GENERIC_READ) == FILE_GENERIC_READ;
					this->pWrite = (grantedAccess & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE;
					this->pExecute = (grantedAccess & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE;
				}
				CloseHandle(hImpersonatedToken);
			}
			CloseHandle(hToken);
		}
	}
	free(security);
#elif defined(PCF_IS_LINUX)
	this->pExists = access(path.string().c_str(), F_OK) == 0;
	this->pRead = access(path.string().c_str(), R_OK) == 0;
	this->pWrite = access(path.string().c_str(), W_OK) == 0;
	this->pExecute = access(path.string().c_str(), X_OK) == 0;
#else /* checking target OS */
#	error "Unsupported target."
#endif
}


} /* namespace path */
} /* namespace pcf */
