/**
 * @file posix_main.cpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-03-09
 * @version 2016-05-01
 */
#include <cstdlib>
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

}

#endif /* windows, unicode */


extern int posix_main(int argc, POSIX_ARG_TYPE ** argv, POSIX_ARG_TYPE ** enpv);


#if defined(PCF_IS_WIN) && defined(_UNICODE)
int wmain(int argc, char ** argv, char ** enpv) {
#else
int main(int argc, char ** argv, char ** enpv) {
#endif
	/* handle Unicode correctly */
#if defined(PCF_IS_WIN) && defined(_UNICODE)
	wchar_t ** wenpv, ** wargv;
	int wargc, si = 0;
	/* this also creates the global variable __wargv */
	__wgetmainargs(&wargc, &wargv, &wenpv, _CRT_glob, &si);
	/* enable UTF-16 output to standard output and standard error console */
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);
	std::locale::global(boost::locale::generator().generate("UTF-8"));
	pcf::coding::Utf8ToUtf16StreamBuffer u8cout(stdout);
	pcf::coding::Utf8ToUtf16StreamBuffer u8cerr(stderr);
	std::streambuf * out = std::cout.rdbuf();
	std::streambuf * err = std::cerr.rdbuf();
	std::cout.rdbuf(&u8cout);
	std::cerr.rdbuf(&u8cerr);
	/* process user defined main function */
	const int result = posix_main(wargc, wargv, wenpv);
	/* revert stream buffers to let cout and cerr clean up remaining memory correctly */
    std::cout.rdbuf(out);
    std::cerr.rdbuf(err);
	return result;
#else /* not windows or unicode */
	std::locale::global(boost::locale::generator().generate("UTF-8"));
	return posix_main(argc, argv, enpv);
#endif /* windows, unicode */
}
