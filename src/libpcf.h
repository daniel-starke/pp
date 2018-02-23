/**
 * @file libpcf.h
 * @author Daniel Starke
 * @copyright Copyright 2010-2018 Daniel Starke
 * @date 2010-01-27
 * @version 2017-01-28
 *
 * The file is used when including the library and to make sure every
 * function is defined correctly for external usage.
 */
#ifndef __LIBPCF_H__
#define __LIBPCF_H__


#if !defined(_WIN32) && !defined(__cdecl)
/** Ignore __cdecl attribute for non Windows platforms. */
#define __cdecl
#endif /* WIN32 */


/**
 * @def HAS_LIBJPEG
 * The macro is defined if libjpeg is available and should
 * be included in the I/O interface.
 * Set INCLUDE_LIBJPEG for the include path of jpeglib.h.
 */
#define HAS_LIBJPEG


/**
 * @def INCLUDE_LIBJPEG
 * The macro defines the include path for jpeglib.h containing
 * the header name as well. To use the libjpeg functions for
 * I/O it is needed to define HAS_LIBJPEG also.
 */
#define INCLUDE_LIBJPEG "jpeglib.h"


/**
 * @def HAS_LIBPNG
 * The macro is defined if libpng is available and should
 * be included in the I/O interface.
 * Set INCLUDE_LIBPNG for the include path of png.h.
 */
#define HAS_LIBPNG


/**
 * @def INCLUDE_LIBPNG
 * The macro defines the include path for png.h containing
 * the header name as well. To use the libpng functions for
 * I/O it is needed to define HAS_LIBPNG also.
 */
#define INCLUDE_LIBPNG "png.h"


/**
 * @def LIBPCF_DECL
 * This macro is used to change the calling convention
 * for the implemented functions.
 */
#undef LIBPCF_DECL
#define LIBPCF_DECL __cdecl


/**
 * @def LIBPCF_DLLPORT
 * This macro is used to change between importing and
 * exporting shared functions which has to be different
 * for building the shared and for using it.
 *
 * @def LIBPCF_DLL
 * This macro is defined by the compiler to adjust the
 * code for shared building.
 *
 * @def LIBPCF_BUILD
 * This macro is defined by the compiler to adjust the
 * code for building the library. Changes are needed to
 * differ between building and using of the header files.
 */
#undef LIBPCF_DLLPORT
#ifdef LIBPCF_DLL
# if __GNUC__ && ! defined (__declspec)
#  error "Please upgrade your GNU compiler to one that supports __declspec."
# endif
# ifdef LIBPCF_BUILD
#  define LIBPCF_DLLPORT __declspec(dllexport)
# else
#  define LIBPCF_DLLPORT __declspec(dllimport)
# endif
#else /* ! defined(LIBPCF_DLL) */
# define LIBPCF_DLLPORT
#endif


#endif /* __LIBPCF_H__ */
