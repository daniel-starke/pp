/**
 * @file typeus.c
 * @author Daniel Starke
 * @copyright Copyright 2014-2018 Daniel Starke
 * @see typeus.h
 * @date 2014-11-08
 * @version 2016-05-01
 */
#include <wchar.h>
#include <stddef.h>
#include <stdint.h>
#include <libpcf/typeus.h>


#include "tusproperty.i"
#include "tusupper.i"
#include "tuslower.i"


/**
 * Returns true if the character is a digit.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a digit, else 0
 */
int tus_isDigit(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_DIGIT)) != 0) ? 1 : 0;
}


/**
 * Returns true if the character is a numeric value.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a numeric value, else 0
 */
int tus_isNumeric(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_NUMERIC)) != 0) ? 1 : 0;
}


/**
 * Returns true if the character is a upper case character.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a upper case character, else 0
 */
int tus_isUpper(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_UPPER)) != 0) ? 1 : 0;
}


/**
 * Returns true if the character is a lower case character.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a lower case character, else 0
 */
int tus_isLower(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_LOWER)) != 0) ? 1 : 0;
}


/**
 * Returns true if the character is alphabetic.
 * 
 * @param[in] var - character to check
 * @return 1 if it is alphabetic, else 0
 */
int tus_isAlpha(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_ALPHA)) != 0) ? 1 : 0;
}


/**
 * Returns true if the character is alphabetic or numeric.
 * 
 * @param[in] var - character to check
 * @return 1 if it is alphabetic or numeric, else 0
 */
int tus_isAlphaNumeric(const wchar_t var) {
	if (tus_isAlpha(var) == 1) return 1;
	if (tus_isNumeric(var) == 1) return 1;
	return 0;
}


/**
 * Returns true if the character is a mathematic symbol.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a mathematic symbol, else 0
 */
int tus_isMath(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_MATH)) != 0) ? 1 : 0;
}


/**
 * Returns true if the character is a hexadecimal digit.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a hexadecimal digit, else 0
 */
int tus_isHex(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_HEX)) != 0) ? 1 : 0;
}


/**
 * Returns true if the character is a whitespace.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a whitespace, else 0
 */
int tus_isWhitespace(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return 0; /* undefined */
	return ((tus_property[value] & ((unsigned int)TUST_WHITESPACE)) != 0) ? 1 : 0;
}


/**
 * Converts a character into its lower case variant.
 * The function only converts if a single character
 * conversion is possible.
 * 
 * @param[in] var - character to convert
 * @return the lower case variant of the character or the character itself
 */
wchar_t tus_toLower(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return var; /* undefined */
	return (wchar_t)tus_lower[value];
}


/**
 * Converts a character into its upper case variant.
 * The function only converts if a single character
 * conversion is possible.
 * 
 * @param[in] var - character to convert
 * @return the upper case variant of the character or the character itself
 */
wchar_t tus_toUpper(const wchar_t var) {
	const unsigned int value = (unsigned int)var;
	if (value > 0xFFFF) return var; /* undefined */
	return (wchar_t)tus_upper[value];
}
