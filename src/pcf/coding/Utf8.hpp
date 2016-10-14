/**
 * @file Utf8.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-10-23
 * @version 2016-05-01
 */
#ifndef __LIBPCFXX_CODING_UTF8_HPP__
#define __LIBPCFXX_CODING_UTF8_HPP__


#include <cstddef>
#include <cstdlib>
#include <locale>
#include <streambuf>
#include <string>
#include <boost/locale.hpp>


namespace pcf {
namespace coding {


size_t countValidUtf8Bytes(const unsigned char * buf, const size_t size);


/**
 * Implements a std::streambuf which interprets written bytes
 * to it as UTF-8 encoded strings and converts them to UTF-16
 * encoded string before outputting it to the file descriptor
 * given.
 */
class Utf8ToUtf16StreamBuffer : public std::basic_streambuf< char, std::char_traits<char> > {
private:
	/** Output buffer size. */
	const size_t bufferSize;
	/** Output buffer. */
	char * outBuf;
	/** Output file descriptor. */
	FILE * outFd;
public:
	/** The size of the input and output buffers. */
	typedef std::char_traits<char> traits_type;
	typedef traits_type::int_type int_type;
	typedef traits_type::pos_type pos_type;
	typedef traits_type::off_type off_type;

	/**
	 * Constructor.
	 *
	 * @param[in] o - output file descriptor
	 * @param[in] bs - buffer size (default: 1024)
	 */
	explicit Utf8ToUtf16StreamBuffer(FILE * o, const size_t bs = 1024) :
		bufferSize(bs),
		outBuf(new char[bs]),
		outFd(o)
	{
		/* Initialize the put pointer. Overflow won't get called until this
		 * buffer is filled up, so we need to use valid pointers.
		 */
		this->setp(outBuf, outBuf + this->bufferSize - 1);
	}

	/** 
	 * Destructor.
	 */
	~Utf8ToUtf16StreamBuffer() {
		delete[] outBuf;
	}
protected:
	virtual int_type overflow(int_type c);
	virtual int_type sync();
};


} /* namespace coding */
} /* namespace pcf */


#endif /* __LIBPCFXX_CODING_UTF8_HPP__ */
