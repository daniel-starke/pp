/**
 * @file fdious.c
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @see fdious.h
 * @date 2015-02-15
 */
#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <libpcf/fdious.h>


/**
 * Defines the size with which a buffer will be increased.
 */
#define BUFFER_INC_SIZE 128


/**
 * The function reads a line and sets the passed string pointer.
 * New space is allocated if the string can not hold more characters.
 *
 * @param[in,out] strPtr - points to the output string
 * @param[in,out] lenPtr - pointer to the size of the input string in characters
 * @param[in] fd - file descriptor to read from
 * @return number of characters read from file descriptor
 */
int fdious_getline(wchar_t ** strPtr, int * lenPtr, FILE * fd) {
	wchar_t * buf, * bufPos, * resStr;
	int totalLen, size, toRead, res;
	if (strPtr == NULL || lenPtr == NULL || fd == NULL) return -1;
	if (*strPtr == NULL || *lenPtr <= 0) {
		*strPtr = (wchar_t *)malloc(sizeof(wchar_t) * BUFFER_INC_SIZE);
		if (*strPtr == NULL) {
			return -1;
		}
		**strPtr = 0;
		*lenPtr = BUFFER_INC_SIZE;
	}
	buf = *strPtr;
	bufPos = buf;
	size = *lenPtr;
	toRead = *lenPtr;
	totalLen = 0;
	while ((resStr = fgetws(bufPos, toRead, fd)) != NULL) {
		res = (int)wcslen(bufPos);
		totalLen += res;
		if ((res + 1) == toRead && bufPos[res - 1] != L'\n') {
			size += BUFFER_INC_SIZE;
			resStr = (wchar_t *)realloc(buf, sizeof(wchar_t) * (size_t)size);
			if (resStr == NULL) {
				if (buf != NULL) free(buf);
				*strPtr = NULL;
				*lenPtr = 0;
				return -1;
			}
			buf = resStr;
			toRead = BUFFER_INC_SIZE + 1;
			bufPos = buf + totalLen;
		} else {
			break;
		}
	}
	if (resStr == NULL) {
		if (feof(fd) != 0) {
			*buf = 0;
			totalLen = 0;
		} else {
			totalLen += (int)wcslen(bufPos);
		}
	}
	*strPtr = buf;
	*lenPtr = size;
	return totalLen;
}


#ifdef PCF_IS_WIN
#define FPOPEN_FUNC fdious_popen
#define FPCLOSE_FUNC fdious_pclose
#define FPOPEN_UNICODE

/* include template function */
#include "fdio.i"
#endif /* PCF_IS_WIN */
