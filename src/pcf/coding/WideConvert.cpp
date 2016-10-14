/**
 * @file WideConvert.cpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-10-19
 * @version 2016-05-01
 */
#include <boost/limits.hpp>
#include <pcf/coding/WideConvert.hpp>


namespace pcf {
namespace coding {


/**
 * Translates sequentially the characters in the range [from, fromEnd)
 * and places them in the range starting at to.
 * It takes the input characters and converts them into wide characters
 * by always taking as many as needed to represent one wide character.
 *
 * @param[in] state - unused
 * @param[in] from - external type, starting position
 * @param[in] fromEnd - external type, end position
 * @param[out] fromNext - external type, next starting position
 * @param[in] to - internal type, starting position
 * @param[in] toEnd - internal type, end position
 * @param[out] toNext - internal type, next starting position
 * @return std::codecvt_base::ok on success, std::codecvt_base::partial if last character is incomplete
 * @see std::codecvt_base
 */
std::codecvt_base::result WideConvert::do_in(
	std::mbstate_t & /* state */,
	const char * from,
	const char * fromEnd,
	const char * & fromNext,
	wchar_t * to,
	wchar_t * toEnd,
	wchar_t * & toNext
) const {
	while (from != fromEnd && to != toEnd) {
		/* If the buffer ends with an incomplete wchar_t character... */
		if ((from + sizeof(wchar_t)) > fromEnd) {
			/* rewind "from" to before the current character translation */
			fromNext = from;
			toNext = to;
			return std::codecvt_base::partial;
		} else {
			*to = *(reinterpret_cast<const wchar_t *>(from));
			from += sizeof(wchar_t);
			to++;
		}
	}

	fromNext = from;
	toNext = to;

	/* Were we done converting or did we run out of destination space? */
	if (from == fromEnd) {
		return std::codecvt_base::ok;
	} else {
		return std::codecvt_base::partial;
	}
}


/**
 * Translates sequentially the characters in the range [from, fromEnd)
 * and places them in the range starting at to.
 * It takes the input wide characters and converts them into characters
 * by always taking as many as needed to represent one wide character.
 * 
 * @param[in] state - unused
 * @param[in] from - internal type, starting position
 * @param[in] fromEnd - internal type, end position
 * @param[out] fromNext - internal type, next starting position
 * @param[in] to - external type, starting position
 * @param[in] toEnd - external type, end position
 * @param[out] toNext - external type, next starting position
 * @return std::codecvt_base::ok on success, std::codecvt_base::partial if last character is incomplete
 * @see std::codecvt_base
 */
std::codecvt_base::result WideConvert::do_out(
	std::mbstate_t & /* state */,
	const wchar_t * from,
	const wchar_t * fromEnd,
	const wchar_t * & fromNext,
	char * to,
	char * toEnd,
	char * & toNext
) const {
	wchar_t maxWchar = (std::numeric_limits<wchar_t>::max)();
	while (from != fromEnd && to != toEnd) {
		// Check for invalid UCS-4 character
		if ((*from) > maxWchar) {
			fromNext = from;
			toNext = to;
			return std::codecvt_base::error;
		}

		/* If the buffer ends with an incomplete wchar_t character... */
		if ((to + sizeof(wchar_t)) > toEnd) {
			/* rewind "from" to before the current character translation */
			fromNext = from;
			toNext = to;
			return std::codecvt_base::partial;
		} else {
			*(reinterpret_cast<wchar_t *>(to)) = *from;
			from++;
			to += sizeof(wchar_t);
		}
	}

	fromNext = from;
	toNext = to;

	/* Were we done converting or did we run out of destination space? */
	if (from == fromEnd) {
		return std::codecvt_base::ok;
	} else {
		return std::codecvt_base::partial;
	}
}


/**
 * Returns the number of external characters in the range [from, fromEnd)
 * that could be translated into at maximum of max internal characters,
 * as if applying codecvt::in.
 *
 * @param[in] state - unused
 * @param[in] from - external type, starting position
 * @param[in] fromEnd - external type, end position
 * @param[in] maxLimit - maximum length of the translated sequence (in terms of internal characters)
 * @return number of characters that can be translated
 */
int WideConvert::do_length(
	std::mbstate_t & /* state */,
	const char * from,
	const char * fromEnd,
	std::size_t maxLimit
) const {
	const std::size_t validLength = static_cast<std::size_t>(fromEnd - from);
	const std::size_t wcharLength = static_cast<std::size_t>(validLength / sizeof(wchar_t));
	
	if (wcharLength > maxLimit) {
		return static_cast<int>(maxLimit * sizeof(wchar_t));
	} else {
		return static_cast<int>(validLength);
	}
}


} /* namespace coding */
} /* namespace pcf */
