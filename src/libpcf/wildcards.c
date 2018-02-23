/**
 * @file wildcards.c
 * @author Daniel Starke
 * @copyright Copyright 2014-2018 Daniel Starke
 * @see wildcards.h
 * @date 2014-11-05
 * @version 2016-05-01
 */
#include <ctype.h>
#include <stddef.h>
#include <libpcf/wildcards.h>


/**
 * Internal function which checks whether the passed
 * character is a digit or not.
 * 
 * @param[in] var - character to check
 * @return 1 if it is a digit, else 0
 * @see isdigit()
 */
static int wcs_matchDigit(const char var) {
	return isdigit((int)var) ? 1 : 0;
}


#define WILDCARD_FUNC wcs_match
#define WILDCARD_NUMBER_FUNC wcs_matchDigit
#define CHAR_T char

/* include template function */
#include "wildcard.i"
