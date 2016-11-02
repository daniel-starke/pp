/**
 * @file posix_main.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-03-09
 * @version 2016-10-28
 */
#ifndef __POSIX_MAIN_HPP__
#define __POSIX_MAIN_HPP__


#include <cstdlib>
#include <pcf/os/Target.hpp>


#if defined(PCF_IS_WIN) && defined(_UNICODE)
#define POSIX_ARG_TYPE wchar_t
#else /* not windows, unicode */
#define POSIX_ARG_TYPE char
#endif /* windows, unicode */


#endif /* __POSIX_MAIN_HPP__ */
