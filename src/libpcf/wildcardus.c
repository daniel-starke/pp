/**
 * @file wildcardus.c
 * @author Daniel Starke
 * @copyright Copyright 2014-2018 Daniel Starke
 * @see wildcardus.h
 * @date 2014-11-05
 * @version 2016-05-01
 */
#include <wchar.h>
#include <stddef.h>
#include <libpcf/typeus.h>
#include <libpcf/wildcardus.h>


#define WILDCARD_FUNC wcus_match
#define WILDCARD_NUMBER_FUNC tus_isNumeric
#define CHAR_T wchar_t

/* include template function */
#include "wildcard.i"
