/**
 * @file libpcfxx-config.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2018 Daniel Starke
 * @date 2014-11-09
 * @version 2017-01-28
 *
 * The file is used to configure some behaviors of
 * the library via per-processor directives.
 */
#ifndef __LIBPCFXX_CONFIG_HPP__
#define __LIBPCFXX_CONFIG_HPP__


#if !defined(_WIN32) && !defined(__cdecl)
/** Ignore __cdecl attribute for non Windows platforms. */
#define __cdecl
#endif /* WIN32 */


/**
 * @def LIBPCF_DECL
 * This macro is used to change the calling convention
 * for the implemented functions.
 */
#undef LIBPCFXX_DECL
#define LIBPCFXX_DECL __cdecl


/**
 * @def LIBPCFXX_PORTABLE
 * Defining this macro will ensure that platform independent
 * code is generated wherever possible.
 * This also includes locale handling.
 * Not defining this will lead to smaller binary size.
 */
//#define LIBPCFXX_PORTABLE 1


#endif /* __LIBPCFXX_HPP__ */
