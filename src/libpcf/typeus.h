/**
 * @file typeus.h
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @see typeus.c
 * @date 2014-11-08
 * @version 2016-05-01
 */
#ifndef __LIBPCF_TYPEUS_H__
#define __LIBPCF_TYPEUS_H__

#include <stdint.h>
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


/**
 * Defines the bits for each item of tus_property.
 */
typedef enum tTUSType {
	TUST_DIGIT      = (1 << 0), /**< is digit */
	TUST_NUMERIC    = (1 << 1), /**< is numeric */
	TUST_UPPER      = (1 << 2), /**< is upper case */
	TUST_LOWER      = (1 << 3), /**< is lower case */
	TUST_ALPHA      = (1 << 4), /**< is alphabetic */
	TUST_MATH       = (1 << 5), /**< is mathematic symbol */
	TUST_HEX        = (1 << 6), /**< is hex digit */
	TUST_WHITESPACE = (1 << 7)  /**< is whitespace */
} tTUSType;


/**
 * Property table for every character in the value range [0x0000, 0xFFFF].
 *
 * @see http://www.unicode.org/Public/UCD/latest/ucdxml/ucd.all.flat.zip
 */
extern const uint8_t tus_property[0x10000];


/**
 * Mapping table to convert characters into their lower case variant.
 *
 * @see http://www.unicode.org/Public/UCD/latest/ucdxml/ucd.all.flat.zip
 */
extern const uint16_t tus_lower[0x10000];


/**
 * Mapping table to convert characters into their upper case variant.
 *
 * @see http://www.unicode.org/Public/UCD/latest/ucdxml/ucd.all.flat.zip
 */
extern const uint16_t tus_upper[0x10000];


/* property check function */
LIBPCF_DLLPORT int LIBPCF_DECL tus_isDigit(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isNumeric(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isUpper(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isLower(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isAlpha(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isAlphaNumeric(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isMath(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isHex(const wchar_t var);
LIBPCF_DLLPORT int LIBPCF_DECL tus_isWhitespace(const wchar_t var);

/* case conversion function */
LIBPCF_DLLPORT wchar_t LIBPCF_DECL tus_toLower(const wchar_t var);
LIBPCF_DLLPORT wchar_t LIBPCF_DECL tus_toUpper(const wchar_t var);


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_TYPEUS_H__ */
