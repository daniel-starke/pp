/**
 * @file posix_main.cpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2017 Daniel Starke
 * @date 2014-03-09
 * @version 2017-01-14
 */
#include <cstdlib>
#include <cstdio>
#include <boost/locale.hpp>
#include <pcf/os/Target.hpp>
#include "posix_main.hpp"


#if defined(PCF_IS_WIN) && defined(_UNICODE)
#include <iostream>
#include <pcf/coding/Utf8.hpp>

extern "C" {

#include <fcntl.h>
#include <io.h>
#include <windows.h>

extern void __wgetmainargs(int *, wchar_t ***, wchar_t ***, int, int *);
#ifdef __MSVCRT__
extern int _CRT_glob;
#else /* ! __MSVCRT__ */
const int _CRT_glob = 0;
#endif /* __MSVCRT__ */

} /* extern "C" */

#endif /* windows, unicode */


extern int posix_main(int argc, POSIX_ARG_TYPE ** argv, POSIX_ARG_TYPE ** enpv);


extern "C" {


#if defined(PCF_IS_WIN) && defined(_UNICODE)
int wmain(int argc, char ** argv, char ** enpv) {
#else
int main(int argc, char ** argv, char ** enpv) {
#endif
	/* handle Unicode correctly */
	std::locale::global(boost::locale::generator().generate(""));
#if defined(PCF_IS_WIN) && defined(_UNICODE)
	wchar_t ** wenpv, ** wargv;
	int wargc, si = 0;
	pcf::coding::Utf8ToUtf16StreamBuffer * u8cout = NULL;
	pcf::coding::Utf8ToUtf16StreamBuffer * u8cerr = NULL;
	std::streambuf * out = NULL;
	std::streambuf * err = NULL;
	/* this also creates the global variable __wargv */
	__wgetmainargs(&wargc, &wargv, &wenpv, _CRT_glob, &si);
	if (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR) {
		/* enable UTF-16 output to standard output console */
		setmode(fileno(stdout), O_U16TEXT);
		u8cout = new pcf::coding::Utf8ToUtf16StreamBuffer(stdout);
		out = std::cout.rdbuf();
		std::cout.rdbuf(u8cout);
	} else {
		/* ensure UTF-8 is output without any transformations */
		setmode(fileno(stdout), O_BINARY);
	}
	if (GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_CHAR) {
		/* enable UTF-16 output to standard error console */
		setmode(fileno(stderr), O_U16TEXT);
		u8cerr = new pcf::coding::Utf8ToUtf16StreamBuffer(stderr);
		err = std::cerr.rdbuf();
		std::cerr.rdbuf(u8cerr);
	} else {
		/* ensure UTF-8 is output without any transformations */
		setmode(fileno(stderr), O_BINARY);
	}
	/* process user defined main function */
	const int result = posix_main(wargc, wargv, wenpv);
	/* revert stream buffers to let cout and cerr clean up remaining memory correctly */
	if (u8cout != NULL) {
		std::cout.rdbuf(out);
		delete u8cout;
	}
	if (u8cerr != NULL) {
		std::cerr.rdbuf(err);
		delete u8cerr;
	}
	return result;
#else /* not windows or unicode */
	return posix_main(argc, argv, enpv);
#endif /* windows, unicode */
}


} /* extern "C" */
