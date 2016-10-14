/**
 * @file wildcards.h
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @see wildcards.c
 * @date 2014-11-05
 * @version 2016-05-01
 */
#ifndef __LIBPCF_WILDCARDS_H__
#define __LIBPCF_WILDCARDS_H__

#include <libpcf.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifndef LIBPCF_DECL
#define LIBPCF_DECL
#endif /* LIBPCF_DECL */

#ifndef LIBPCF_DLLPORT
#define LIBPCF_DLLPORT
#endif /* LIBPCF_DLLPORT */


LIBPCF_DLLPORT int LIBPCF_DECL wcs_match(const char * text, const char * pattern);


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_WILDCARDS_H__ */
