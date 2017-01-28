/**
 * @file OctetBlock.hpp
 * @author Daniel Starke
 * @copyright Copyright 2013-2017 Daniel Starke
 * @date 2013-06-25
 * @version 2016-11-20
 * @see http://2π.com/11/variable-sized-integers
 */
#ifndef __LIBPCFXX_DATA_OCTETBLOCK_HPP__
#define __LIBPCFXX_DATA_OCTETBLOCK_HPP__

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <vector>
#include <sstream>
#include <string>
#include <boost/cstdint.hpp>
#include <libpcfxx.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/os/Target.hpp>


namespace pcf {
namespace data {


/**
 * Data manipulation class with additional vector methods to process
 * octet data arrays.
 * 
 * @remarks Variable integers are encoded as in EBML.
 */
class OctetBlock : public std::vector<boost::uint8_t> {
public:
	/** Base class type. */
	typedef std::vector<boost::uint8_t> Base;
	
	/**
	 * Inherit base constructor from std::vector<boost::uint8_t>.
	 */
	explicit OctetBlock(const Base::allocator_type & alloc = Base::allocator_type()) :
		Base(alloc)
	{}
	
	/**
	 * Inherit base constructor from std::vector<boost::uint8_t>.
	 */
	explicit OctetBlock(Base::size_type n, const Base::value_type & val = Base::value_type(), const Base::allocator_type & alloc = Base::allocator_type()) :
		Base(n, val, alloc)
	{}
	
	/**
	 * Inherit base constructor from std::vector<boost::uint8_t>.
	 */
	template <typename InputIterator>
	OctetBlock(InputIterator first, InputIterator last, const Base::allocator_type & alloc = Base::allocator_type()) :
		Base(first, last, alloc)
	{}
	
	/**
	 * Inherit base constructor from std::vector<boost::uint8_t>.
	 */
	OctetBlock(const vector & x) :
		Base(x)
	{}
	
	/**
	 * Performs an unchecked little-endian copy from the given position to the destination memory.
	 *
	 * @tparam T - output memory type
	 * @param[in] index - index to the first octet starting at 0
	 * @param[in] dataSize - size to copy in octets
	 * @param[out] dst - copy to the memory pointed here
	 */
	template <typename T>
	void getCopyLE(const Base::size_type index, const Base::size_type dataSize, T * dst) const {
		std::copy(
			Base::begin() + static_cast<Base::difference_type>(index),
			Base::begin() + static_cast<Base::difference_type>(index + dataSize),
			reinterpret_cast<boost::uint8_t *>(dst)
		);
	}
	
	/**
	 * Performs an unchecked big-endian copy from the given position to the destination memory.
	 *
	 * @tparam T - output memory type
	 * @param[in] index - index to the first octet starting at 0
	 * @param[in] dataSize - size to copy in octets
	 * @param[out] dst - copy to the memory pointed here
	 */
	template <typename T>
	void getCopyBE(const Base::size_type index, const Base::size_type dataSize, T * dst) const {
		std::reverse_copy(
			Base::begin() + static_cast<Base::difference_type>(index),
			Base::begin() + static_cast<Base::difference_type>(index + dataSize),
			reinterpret_cast<boost::uint8_t *>(dst)
		);
	}
	
	/**
	 * Performs an unchecked big-endian copy from the given position as variable integer.
	 * The value is assumed to be encoded according to the EBML specification.
	 *
	 * @param[in] index - index to the first octet starting at 0
	 * @param[in] octetCount - number of octets to copy
	 * @param[in] firstOctet - first octet (including bit prefix)
	 * @return variable integer at position index
	 */
	boost::uint64_t getUncheckedVarIntBE(const Base::size_type index, const size_t octetCount, const Base::value_type firstOctet) const {
		boost::uint64_t result;
		result = static_cast<boost::uint64_t>(firstOctet & (0xFF >> octetCount));
		for (size_t i = 1; i < octetCount; i++) {
			result = (result << 8) | static_cast<boost::uint64_t>(Base::operator[] (index + i));
		}
		return result;
	}
	
	/**
	 * Performs an unchecked big-endian copy from variable integer to the given position.
	 * The value is assumed to be encoded according to the EBML specification.
	 *
	 * @param[in] index - index to the first octet starting at 0
	 * @param[in] octetCount - number of octets to copy
	 * @param[in] value - variable integer value
	 */
	void setUncheckedVarIntBE(const Base::size_type index, const size_t octetCount, const boost::uint64_t value) {
		const boost::uint64_t mask(0xFF >> octetCount);
		boost::uint64_t tmp(value);
		Base::operator[] (index) = static_cast<Base::value_type>(1 << (8 - octetCount));
		for (size_t i = 1; i < octetCount; i++) {
			Base::operator[] (index + octetCount - i) = static_cast<Base::value_type>(tmp & 0xFF);
			tmp >>= 8;
		}
		Base::operator[] (index) |= static_cast<Base::value_type>(tmp & mask);
	}
	
	/**
	 * The method returns the passed type from the given index in little-endian format.
	 *
	 * @tparam T - return type
	 * @param[in] index - index to the first octet starting at 0
	 * @return value at position index of type T
	 */
	template <typename T>
	const T getLE(const Base::size_type index) const {
		T result;
		if ((index + sizeof(T)) > Base::size()) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Trying to read beyond the end.")
			);
		}
		this->getCopyLE(index, sizeof(T), &result);
		return result;
	}
	
	/**
	 * The method returns the variable integer from the given index in
	 * big-endian format.
	 * The value is assumed to be encoded according to the EBML specification.
	 *
	 * @param[in] index - index to the first octet starting at 0
	 * @return variable integer at position index
	 */
	boost::uint64_t getVarIntBE(const Base::size_type index) const {
		using namespace pcf::os;
		const size_t currentSize(Base::size());
		if ((index + 1) > currentSize) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Trying to read beyond the end.")
			);
		}
		const Base::value_type firstOctet(Base::operator[] (index));
		const size_t octetCount = static_cast<size_t>(PCF_LEADING_ZEROS32(static_cast<boost::uint32_t>(firstOctet))) - 23;
		if ((index + octetCount) > currentSize) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Trying to read beyond the end.")
			);
		}
		return this->getUncheckedVarIntBE(index, octetCount, firstOctet);
	}
	
	/**
	 * The method returns the passed type from the given index in big-endian format.
	 *
	 * @tparam T - return type
	 * @param[in] index - index to the first octet starting at 0
	 * @return value at position index of type T
	 */
	template <typename T>
	const T getBE(const Base::size_type index) const {
		T result;
		if ((index + sizeof(T)) > Base::size()) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Trying to read beyond the end.")
			);
		}
		this->getCopyBE(index, sizeof(T), &result);
		return result;
	}
	
	/**
	 * Performs an unchecked little-endian copy from the source memory to the given position.
	 *
	 * @tparam T - input memory type
	 * @param[in] index - index to the first octet starting at 0
	 * @param[in] dataSize - size to copy in octets
	 * @param[out] src - copy from the memory pointed here
	 */
	template <typename T>
	void setCopyLE(const Base::size_type index, const Base::size_type dataSize, T * src) {
		std::copy(
			reinterpret_cast<const boost::uint8_t *>(src),
			reinterpret_cast<const boost::uint8_t *>(src) + dataSize,
			Base::begin() + static_cast<Base::difference_type>(index)
		);
	}
	
	/**
	 * Performs an unchecked big-endian copy from the source memory to the given position.
	 *
	 * @tparam T - input memory type
	 * @param[in] index - index to the first octet starting at 0
	 * @param[in] dataSize - size to copy in octets
	 * @param[out] src - copy from the memory pointed here
	 */
	template <typename T>
	void setCopyBE(const Base::size_type index, const Base::size_type dataSize, T * src) {
		std::reverse_copy(
			reinterpret_cast<const boost::uint8_t *>(src),
			reinterpret_cast<const boost::uint8_t *>(src) + dataSize,
			Base::begin() + static_cast<Base::difference_type>(index)
		);
	}
	
	/**
	 * The method sets the OctetBlock at the given position to the octet stream of
	 * the passed value in little-endian format.
	 *
	 * @tparam T - value type
	 * @param[in] index - OctetBlock position to set the value at
	 * @param[in] value - value to set
	 */
	template <typename T>
	void setLE(const Base::size_type index, const T & value) {
		if ((index + sizeof(T)) > Base::size()) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Trying to write beyond the end.")
			);
		}
		this->setCopyLE(index, sizeof(T), &value);
	}
	
	/**
	 * The method sets the OctetBlock at the given position to the octet stream of
	 * the passed value in little-endian format.
	 *
	 * @param[in] index - OctetBlock position to set the value at
	 * @param[in] value - char array to set
	 */
	void setLE(const Base::size_type index, const char * value);
	
	/**
	 * The method sets the OctetBlock at the given position to the octet stream of
	 * the passed value in big-endian format.
	 *
	 * @tparam T - value type
	 * @param[in] index - OctetBlock position to set the value at
	 * @param[in] value - value to set
	 */
	template <typename T>
	void setBE(const Base::size_type index, const T & value) {
		if ((index + sizeof(T)) > Base::size()) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Trying to write beyond the end.")
			);
		}
		this->setCopyBE(index, sizeof(T), &value);
	}
	
	/**
	 * The method sets the OctetBlock at the given position to the octet stream of
	 * the passed value in big-endian format.
	 *
	 * @param[in] index - OctetBlock position to set the value at
	 * @param[in] value - char array to set
	 */
	void setBE(const Base::size_type index, const char * value);
	
	/**
	 * The method sets the OctetBlock at the given position to the variable integer of
	 * the passed value in big-endian format.
	 * The value is assumed to be encoded according to the EBML specification.
	 *
	 * @param[in] index - OctetBlock position to set the value at
	 * @param[in] value - value to set
	 * @return set number of octets
	 */
	size_t setVarIntBE(const Base::size_type index, const boost::uint64_t value) {
		const size_t octetCount(OctetBlock::getVarIntSize(value));
		if ((index + octetCount) > Base::size()) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Trying to write beyond the end.")
			);
		}
		this->setUncheckedVarIntBE(index, octetCount, value);
		return octetCount;
	}
	
	/**
	 * The method removes and returns the value from the end of the octet block in little-endian format.
	 *
	 * @tparam T - value type
	 * @return value from the end of type T
	 */
	template <typename T>
	const T popBackLE() {
		const Base::size_type newSize = Base::size() - sizeof(T);
		T result;
		if (Base::size() < sizeof(T)) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Not enough data stored to complete.")
			);
		}
		this->getCopyLE(newSize, sizeof(T), &result);
		Base::resize(newSize);
		return result;
	}
	
	/**
	 * The method removes and returns the value from the end of the octet block in big-endian format.
	 *
	 * @tparam T - value type
	 * @return value from the end of type T
	 */
	template <typename T>
	const T popBackBE() {
		const Base::size_type newSize = Base::size() - sizeof(T);
		T result;
		if (Base::size() < sizeof(T)) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::OutOfRange()
				<< pcf::exception::tag::Message("Not enough data stored to complete.")
			);
		}
		this->getCopyBE(newSize, sizeof(T), &result);
		Base::resize(newSize);
		return result;
	}
	
	/**
	 * The method removes and returns the variable integer value from the end
	 * of the octet block in big-endian format.
	 * The value is assumed to be encoded according to the EBML specification.
	 *
	 * @return variable integer value from the end
	 */
	boost::uint64_t popBackVarIntBE() {
		using namespace pcf::os;
		const Base::size_type currentSize = Base::size();
		boost::uint64_t result(0);
		bool found(false);
		for (size_t i = 1; i <= 9; i++) {
			if (i > currentSize) break;
			const Base::value_type firstOctet(Base::operator[] (currentSize - i));
			const size_t octetCount = static_cast<size_t>(PCF_LEADING_ZEROS32(static_cast<boost::uint32_t>(firstOctet))) - 23;
			if (octetCount == i) {
				const uint64_t tmp = this->getUncheckedVarIntBE(currentSize - i, octetCount, firstOctet);
				if (tmp > result) {
					result = tmp;
					Base::resize(currentSize - i);
					found = true;
				}
			}
		}
		if ( ! found ) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::InvalidValue()
				<< pcf::exception::tag::Message("No valid variable integer found at the end of the OctetBlock.")
			);
		}
		return result;
	}
	
	/**
	 * The method adds a value to the end of the block in little-endian format.
	 *
	 * @tparam T - value type
	 * @param[in] value - value to add
	 */
	template <typename T>
	void pushBackLE(const T & value) {
		const Base::size_type baseSize = Base::size();
		Base::resize(baseSize + sizeof(T));
		this->setCopyLE(baseSize, sizeof(T), &value);
	}
	
	/**
	 * The method adds a char array to the end of the block in little-endian format.
	 *
	 * @param[in] value - char array to add
	 */
	void pushBackLE(const char * value);
	
	/**
	 * The method adds a value to the end of the block in big-endian format.
	 *
	 * @tparam T - value type
	 * @param[in] value - value to add
	 */
	template <typename T>
	void pushBackBE(const T & value) {
		const Base::size_type baseSize = Base::size();
		Base::resize(baseSize + sizeof(T));
		this->setCopyBE(baseSize, sizeof(T), &value);
	}
	
	/**
	 * The method adds a char array to the end of the block in big-endian format.
	 *
	 * @param[in] value - char array to add
	 */
	void pushBackBE(const char * value);
	
	/**
	 * The method adds a variable integer value to the end of the
	 * block in big-endian format.
	 * The value is assumed to be encoded according to the EBML specification.
	 *
	 * @param[in] value - variable integer value to add
	 */
	void pushBackVarIntBE(const boost::uint64_t & value) {
		const size_t octetCount(OctetBlock::getVarIntSize(value));
		const Base::size_type baseSize = Base::size();
		Base::resize(baseSize + octetCount);
		this->setUncheckedVarIntBE(baseSize, octetCount, value);
	}
	
	/**
	 * This method appends another OctetBlock at the end of this OctetBlock object.
	 *
	 * @param[in] value - OctetBlock to append
	 * @return reference to this object for chained operation
	 */
	OctetBlock & append(const OctetBlock & value) {
		Base::insert(Base::end(), value.Base::begin(), value.Base::end());
		return *this;
	}
	
	/**
	 * This method appends the decoded hex string data at the
	 * end of this OctetBlock object.
	 *
	 * @param[in] value - hex string to append
	 * @return reference to this object for chained operation
	 */
	OctetBlock & append(const std::string & value) {
		std::string::const_iterator it, endIt;
		Base::value_type octet, nibble;
		bool first;
		endIt = value.end();
		first = true;
		for (it = value.begin(); it < endIt; it++) {
			const std::string::value_type c = *it;
			if (c >= '0' && c <= '9') {
				nibble = static_cast<Base::value_type>(c - '0');
			} else if (c >= 'a' && c <= 'f') {
				nibble = static_cast<Base::value_type>(c - 'a' + 10);
			} else if (c >= 'A' && c <= 'F') {
				nibble = static_cast<Base::value_type>(c - 'A' + 10);
			} else {
				continue;
			}
			if ( first ) {
				octet = nibble;
				first = false;
			} else {
				this->Base::push_back(static_cast<Base::value_type>((octet << 4) | nibble));
				first = true;
			}
		}
		return *this;
	}
	
	/**
	 * Returns a hex dump of the whole content.
	 *
	 * @param[in] maxPerLine - optional limit number of octet hex values per line or set to 0
	 * @param[in] prefix - optional string prefix before each octet hex value
	 * @return hex dump as string
	 */
	const std::string hexDump(const Base::size_type maxPerLine, const std::string prefix) const {
		Base::const_iterator it;
		const Base::const_iterator endIt = Base::end();
		std::ostringstream sout;
		Base::size_type count = 0;
		bool first = true;
		for (it = Base::begin(); it < endIt; it++) {
			if ( ! first ) sout << ' ';
			first = false;
			count++;
			sout << prefix << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(*it);
			if (maxPerLine > 0 && count >= maxPerLine) {
				sout << std::endl;
				first = true;
				count = 0;
			}
		}
		if (maxPerLine > 0 && first == false) {
			sout << std::endl;
		}
		return sout.str();
	}
	
	/**
	 * Returns a hex dump of the whole content.
	 *
	 * @param[in] prefix - optional string prefix before each octet hex value
	 * @return hex dump as string
	 */
	const std::string hexDump(const std::string prefix = std::string()) const {
		return this->hexDump(0, prefix);
	}
	
	/**
	 * Returns a hex dump of the whole content.
	 *
	 * @param[in] maxPerLine - optional limit number of octet hex values per line or set to 0
	 * @return hex dump as string
	 */
	const std::string hexDump(const Base::size_type maxPerLine) const {
		return this->hexDump(maxPerLine, std::string());
	}
	
	/**
	 * Returns a hex dump of the whole content with the format:
	 * @pre
	 * 00000000 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
	 * offset   hex data       	                                ASCII data
	 * @endpre
	 *
	 * @param[in] maxPerLine - optional limit of octet hex values per line (default is 16)
	 * @param[in] initOffset - optional initial offset (default is 0)
	 * @return standard hex dump
	 */
	const std::string stdHexDump(const Base::size_type maxPerLine = 16, const Base::size_type initOffset = 0) const {
		Base::const_iterator it;
		const Base::const_iterator endIt = Base::end();
		std::ostringstream sout;
		std::ostringstream sAscii;
		Base::size_type count = 0;
		Base::size_type offset = initOffset;
		bool first = true;
		/* check constraints */
		if (maxPerLine < 1) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::InvalidValue()
				<< pcf::exception::tag::Message("Invalid maximum number of octets per line.")
			);
		}
		/* output full lines */
		for (it = Base::begin(); it < endIt; it++) {
			if ( first ) {
				sout << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << offset;
			}
			first = false;
			count++;
			sout << ' ' << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(*it);
			if ( std::isprint(*it) ) {
				sAscii << *it;
			} else {
				sAscii << '.';
			}
			if (count >= maxPerLine) {
				sout << ' ' << sAscii.str() << std::endl;
				first = true;
				offset += count;
				count = 0;
				sAscii.str(std::string());
			}
		}
		/* output remaining part */
		if ( ! first ) {
			for (; count < maxPerLine; count++) {
				sout << " ..";
			}
			sout << ' ' << sAscii.str() << std::endl;
		}
		return sout.str();
	}
	
	/**
	 * Converts the data to a continues hex representation.
	 *
	 * @return hex string
	 */
	const std::string hexString() const {
		Base::const_iterator it;
		const Base::const_iterator endIt = Base::end();
		std::ostringstream sout;
		for (it = Base::begin(); it < endIt; it++) {
			sout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(*it);
		}
		return sout.str();
	}
public:
	/**
	 * Returns the size in octet of the passed value encoded as variable integer.
	 *
	 * @param[in] value - determine size of this value
	 * @return stored size in octets
	 */
	static size_t getVarIntSize(const boost::uint64_t value) {
		using namespace pcf::os;
		if (value >= UINT64_C(0x8000000000000000)) return 9;
		return static_cast<const size_t>(9 - ((PCF_LEADING_ZEROS64(value) - 1) / 7));
	}
};


template <> void OctetBlock::pushBackLE(const std::string & value);
template <> void OctetBlock::pushBackBE(const std::string & value);
template <> const std::string OctetBlock::popBackLE();
template <> const std::string OctetBlock::popBackBE();
template <> const std::string OctetBlock::getLE(const Base::size_type index) const;
template <> const std::string OctetBlock::getBE(const Base::size_type index) const;
template <> void OctetBlock::setLE(const Base::size_type index, const std::string & value);
template <> void OctetBlock::setBE(const Base::size_type index, const std::string & value);


} /* namespace data */
} /* namespace pcf */


#endif /* __LIBPCFXX_DATA_OCTETBLOCK_HPP__ */
