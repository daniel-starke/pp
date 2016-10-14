/**
 * @file wildcardus.h
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @see wildcardus.c
 * @date 2014-11-05
 * @version 2016-05-01
 */
#ifndef __LIBPCF_WILDCARDUS_H__
#define __LIBPCF_WILDCARDUS_H__

#include <wchar.h>
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


LIBPCF_DLLPORT int LIBPCF_DECL wcus_match(const wchar_t * text, const wchar_t * pattern);


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_WILDCARDUS_H__ */
