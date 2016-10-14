/**
 * @file fdio.c
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @see fdio.h
 * @date 2015-02-28
 * @version 2016-05-01
 */
#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <libpcf/target.h>
#include <libpcf/fdio.h>

#ifdef PCF_IS_WIN
#include <fcntl.h>
#include <io.h>
#endif /* PCF_IS_WIN */


/**
 * Sets the binary/text mode of a given file descriptor.
 *
 * @param[in,out] fd - file descriptor to change
 * @param[in] mode - set this mode
 * @return 1 on success, else 0
 * @see tFdioMode
 */
int fdio_setMode(FILE * fd, tFdioMode mode) {
#ifdef PCF_IS_WIN
	switch (mode) {
	case FDIO_TEXT:
		return _setmode(_fileno(fd), _O_TEXT) == -1 ? 0 : 1;
		break;
	case FDIO_WTEXT:
		return _setmode(_fileno(fd), _O_WTEXT) == -1 ? 0 : 1;
		break;
	case FDIO_UTF8:
		return _setmode(_fileno(fd), _O_U8TEXT) == -1 ? 0 : 1;
		break;
	case FDIO_UTF16:
		return _setmode(_fileno(fd), _O_U16TEXT) == -1 ? 0 : 1;
		break;
	case FDIO_BINARY:
		return _setmode(_fileno(fd), _O_BINARY) == -1 ? 0 : 1;
		break;
	}
#else /* ! PCF_IS_WIN */
	/* it is handled in all binary mode in Linux */
	PCF_UNUSED(fd);
	PCF_UNUSED(mode);
	return 1;
#endif /* PCF_IS_WIN */
	return 0;
}
