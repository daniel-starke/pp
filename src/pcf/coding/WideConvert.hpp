/**
 * @file WideConvert.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-10-19
 * @version 2016-05-01
 */
#ifndef __LIBPCFXX_CODING_WIDECONVERT_HPP__
#define __LIBPCFXX_CODING_WIDECONVERT_HPP__


#include <cwchar>
#include <cstddef>
#include <locale>


namespace pcf {
namespace coding {


/**
 * Class to interpret a character stream as wide characters.
 */
class WideConvert : public std::codecvt<wchar_t, char, std::mbstate_t> {
public:
	typedef wchar_t   intern_type; /**< internal character type */
	typedef char      extern_type; /**< external character type (translated to and from this type) */
	typedef mbstate_t state_type; /**< type of the conversion state */

	/**
	 * Constructor.
	 *
	 * @param[in] noLocaleManage - set to 0 if object shall be automatically deleted, set to 1 else
	 */
	explicit WideConvert(std::size_t noLocaleManage = 0) :
		std::codecvt<wchar_t, char, std::mbstate_t>(noLocaleManage)
	{}

protected:
	virtual std::codecvt_base::result do_in(
		std::mbstate_t & /* state */,
		const char * from,
		const char * fromEnd,
		const char * & fromNext,
		wchar_t * to,
		wchar_t * toEnd,
		wchar_t * & toNext
	) const;

	virtual std::codecvt_base::result do_out(
		std::mbstate_t & /* state */,
		const wchar_t * from,
		const wchar_t * fromEnd,
		const wchar_t * & fromNext,
		char * to,
		char * toEnd,
		char * & toNext
	) const;

	/**
	 * Always returns false.
	 *
	 * @param[in] octet1 - unused
	 * @return false
	 */
	bool invalid_continuing_octet(unsigned char /* octet1 */) const {
		return false;
	}

	/**
	 * Always returns false.
	 *
	 * @param[in] octet1 - unused
	 * @return false
	 */
	bool invalid_leading_octet(unsigned char /* octet1 */) const {
		return false;
	}

	/**
	 * Always returns size of wchar_t - 1.
	 *
	 * @param[in] leadOctet - unused
	 * @return size of wchar_t - 1
	 */
	static unsigned int get_cont_octet_count(unsigned char leadOctet) {
		return get_octet_count(leadOctet) - 1;
	}

	/**
	 * Always returns size of wchar_t.
	 *
	 * @param[in] leadOctet - unused
	 * @return size of wchar_t
	 */
	static unsigned int get_octet_count(unsigned char /* leadOctet */) {
		return static_cast<unsigned int>(sizeof(wchar_t));
	}

	/**
	 * Always returns size of wchar_t - 1.
	 *
	 * @param[in] leadOctet - unused
	 * @return size of wchar_t - 1
	 */
	int get_cont_octet_out_count(wchar_t /* word */) const {
		return static_cast<int>(sizeof(wchar_t)) - 1;
	}

	/**
	 * Always returns false.
	 *
	 * @return false
	 */
	virtual bool do_always_noconv() const throw() {
		return false;
	}

	/**
	 * Sets next position to from position.
	 *
	 * @param[in] state - unused
	 * @param[in] from - new starting position
	 * @param[in] to - unused
	 * @param[out] next - reference to current starting position
	 * @return std::codecvt_base::ok
	 */
	virtual std::codecvt_base::result do_unshift(
		std::mbstate_t & /* state */,
		char * from,
		char * /* to */,
		char * & next
	) const
	{
		next = from;
		return ok;
	}

	/**
	 * Always returns 0.
	 *
	 * @return 0
	 */
	virtual int do_encoding() const throw() {
		const int variable_byte_external_encoding = 0;
		return variable_byte_external_encoding;
	}

	virtual int do_length(
		std::mbstate_t & /* state */,
		const char * from,
		const char * fromEnd,
		std::size_t maxLimit
	) const;

	/**
	 * Always returns size of wchar_t.
	 *
	 * @return size of wchar_t
	 */
	virtual int do_max_length() const throw() {
		return static_cast<int>(sizeof(wchar_t));
	}
};


} /* namespace coding */
} /* namespace pcf */


#endif /* __LIBPCFXX_CODING_WIDECONVERT_HPP__ */
