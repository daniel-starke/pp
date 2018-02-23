/**
 * @file Utf8.cpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2018 Daniel Starke
 * @date 2014-10-23
 * @version 2016-10-28
 */
#include <pcf/coding/Utf8.hpp>


namespace pcf {
namespace coding {


/**
 * Counts the number of valid bytes for a UTF-8 encoded
 * string. This excludes incomplete UTF-8 characters at
 * the end of the string.
 * The real input buffer size is not checked additionally
 * which means that size shell not be larger than the
 * given buffer.
 *
 * @param[in] buf - input string
 * @param[in] size - size of the input string
 * @return number of valid UTF-8 bytes
 */
size_t countValidUtf8Bytes(const unsigned char * buf, const size_t size) {
	size_t i, charSize;
	const unsigned char * src = buf;
	for (i = 0; i < size && (*src) != 0; i += charSize, src += charSize) {
		charSize = 0;
		if ((*src) >= 0xFC) {
			charSize = 6;
		} else if ((*src) >= 0xF8) {
			charSize = 5;
		} else if ((*src) >= 0xF0) {
			charSize = 4;
		} else if ((*src) >= 0xE0) {
			charSize = 3;
		} else if ((*src) >= 0xC0) {
			charSize = 2;
		} else if ((*src) >= 0x80) {
			/* Skip continuous UTF-8 character (should never happen). */
			for (; (i + charSize) < size && src[charSize] != 0 && src[charSize] >= 0x80; charSize += 2);
		} else {
			/* ASCII character. */
			charSize = 1;
		}
		if ((i + charSize) > size) break;
	}
	return i;
}


/**
 * Called when the buffer is filled up.
 * It converts the output from UTF-8 to UTF-16 and writes it
 * to configured file descriptor.
 *
 * @param[in] c - character to add
 * @return added character on success, else EOF
 */
Utf8ToUtf16StreamBuffer::int_type Utf8ToUtf16StreamBuffer::overflow(Utf8ToUtf16StreamBuffer::int_type c) {
	char * iBegin = this->outBuf;
	char * iEnd = this->pptr();
	int_type result = traits_type::not_eof(c);

	/* If this is the end, add an eof character to the buffer.
	 * This is why the pointers passed to setp are off by 1
	 * (to reserve room for this).
	 */
	if ( ! traits_type::eq_int_type(c, traits_type::eof()) ) {
		*iEnd = traits_type::to_char_type(c);
		iEnd++;
	}

	/* Calculate output data length. */
	int_type iLen = static_cast<int_type>(iEnd - iBegin);
	int_type iLenU8 = static_cast<int_type>(
		countValidUtf8Bytes(reinterpret_cast<const unsigned char *>(iBegin), static_cast<size_t>(iLen))
	);

	/* Convert string to UTF-16 and write to defined file descriptor. */
	const std::wstring outStr(boost::locale::conv::utf_to_utf<wchar_t>(std::string(outBuf, outBuf + iLenU8)));
#ifdef _MSC_VER
	if (fwprintf(this->outFd, L"%s", outStr.c_str()) < 0) {
#else
	if (fwprintf(this->outFd, L"%S", outStr.c_str()) < 0) {
#endif
		/* Failed to write data to output file descriptor. */
		result = traits_type::eof();
	}

	/* Reset the put pointers to indicate that the buffer is free. */
	if (iLenU8 == iLen) {
		this->setp(outBuf, outBuf + this->bufferSize + 1);
	} else {
		/* Move incomplete UTF-8 characters remaining in buffer. */
		const size_t overhead = static_cast<size_t>(iLen - iLenU8);
		memmove(outBuf, outBuf + iLenU8, overhead);
		this->setp(outBuf + overhead, outBuf + this->bufferSize + 1);
	}

	return result;
}

/**
 * This is called to flush the buffer.
 * It is called at the end of the file stream or when flush() is called.
 *
 * @return 0 on success, else -1
 */
Utf8ToUtf16StreamBuffer::int_type Utf8ToUtf16StreamBuffer::sync() {
	return traits_type::eq_int_type(this->overflow(traits_type::eof()), traits_type::eof()) ? -1 : 0;
}


} /* namespace coding */
} /* namespace pcf */
