/**
 * @file libpcfxx.hpp
 * @author Daniel Starke
 * @copyright Copyright 2013-2018 Daniel Starke
 * @date 2013-06-25
 * @version 2016-05-01
 *
 * The file is used when including the library and to make sure every
 * function is defined correctly for external usage.
 */
#ifndef __LIBPCFXX_HPP__
#define __LIBPCFXX_HPP__


#include <libpcfxx-config.hpp>


/**
 * @def LIBPCFXX_DLLPORT
 * This macro is used to change between importing and
 * exporting dll functions which has to be different
 * for building the dll and for using it.
 *
 * @def LIBPCFXX_DLL
 * This macro is defined by the compiler to adjust the
 * code for dll building.
 *
 * @def LIBPCFXX_BUILD
 * This macro is defined by the compiler to adjust the
 * code for building the library. Changes are needed to
 * differ between building and using of the header files.
 */
#undef LIBPCFXX_DLLPORT
#ifdef LIBPCFXX_DLL
# if __GNUC__ && ! defined (__declspec)
#  error "Please upgrade your GNU compiler to one that supports __declspec."
# endif
# ifdef LIBPCFXX_BUILD
#  define LIBPCFXX_DLLPORT __declspec(dllexport)
# else
#  define LIBPCFXX_DLLPORT __declspec(dllimport)
# endif
#else /* ! defined(LIBPCFXX_DLL) */
# define LIBPCFXX_DLLPORT
#endif


#endif /* __LIBPCFXX_HPP__ */
