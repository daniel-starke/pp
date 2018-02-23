/**
 * @file OctetBlock.cpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2018 Daniel Starke
 * @date 2014-01-23
 * @version 2016-05-01
 */
#include <pcf/data/OctetBlock.hpp>


namespace pcf {
namespace data {


template <>
void OctetBlock::pushBackLE(const std::string & value) {
	const Base::size_type baseSize = Base::size();
	const boost::uint32_t stringSize = static_cast<const boost::uint32_t>(value.size());
	Base::resize(baseSize + sizeof(boost::uint32_t) + (stringSize * sizeof(std::string::value_type)));
	this->setCopyLE(baseSize, sizeof(boost::uint32_t), &stringSize);
	this->setCopyLE(baseSize + sizeof(boost::uint32_t), stringSize, value.data());
}


void OctetBlock::pushBackLE(const char * value) {
	this->pushBackLE(std::string(value));
}


template <>
void OctetBlock::pushBackBE(const std::string & value) {
	const Base::size_type baseSize = Base::size();
	const boost::uint32_t stringSize = static_cast<const boost::uint32_t>(value.size());
	Base::resize(baseSize + sizeof(boost::uint32_t) + (stringSize * sizeof(std::string::value_type)));
	this->setCopyBE(baseSize, sizeof(boost::uint32_t), &stringSize);
	this->setCopyLE(baseSize + sizeof(boost::uint32_t), stringSize, value.data());
}


void OctetBlock::pushBackBE(const char * value) {
	this->pushBackBE(std::string(value));
}


template <>
const std::string OctetBlock::popBackLE() {
	const Base::size_type baseSize = Base::size();
	std::string result;
	boost::uint32_t stringSize;
	Base::size_type i;
	if (sizeof(boost::uint32_t) > baseSize) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Not enough data stored to complete.")
		);
	}
	for (i = (baseSize - sizeof(boost::uint32_t)); (i + 1) > 0; i--) {
		this->getCopyLE(i, sizeof(boost::uint32_t), &stringSize);
		if (stringSize == (baseSize - (i + sizeof(boost::uint32_t)))) {
			result.resize(stringSize);
			std::copy(
				Base::begin() + static_cast<Base::difference_type>(i + sizeof(boost::uint32_t)),
				Base::begin() + static_cast<Base::difference_type>(i + sizeof(boost::uint32_t) + stringSize),
				result.begin()
			);
			Base::resize(i);
			return result;
		}
	}
	BOOST_THROW_EXCEPTION(
		pcf::exception::DataNotFound()
		<< pcf::exception::tag::Message("No valid string found.")
	);
	return result;
}


template <>
const std::string OctetBlock::popBackBE() {
	const Base::size_type baseSize = Base::size();
	std::string result;
	boost::uint32_t stringSize;
	Base::size_type i;
	if (sizeof(boost::uint32_t) > baseSize) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Not enough data stored to complete.")
		);
	}
	for (i = (baseSize - sizeof(boost::uint32_t)); (i + 1) > 0; i--) {
		this->getCopyBE(i, sizeof(boost::uint32_t), &stringSize);
		if (stringSize == (baseSize - (i + sizeof(boost::uint32_t)))) {
			result.resize(stringSize);
			std::copy(
				Base::begin() + static_cast<Base::difference_type>(i + sizeof(boost::uint32_t)),
				Base::begin() + static_cast<Base::difference_type>(i + sizeof(boost::uint32_t) + stringSize),
				result.begin()
			);
			Base::resize(i);
			return result;
		}
	}
	BOOST_THROW_EXCEPTION(
		pcf::exception::DataNotFound()
		<< pcf::exception::tag::Message("No valid string found.")
	);
	return result;
}


template <>
const std::string OctetBlock::getLE(const Base::size_type index) const {
	const Base::size_type baseSize = Base::size();
	std::string result;
	boost::uint32_t stringSize;
	if ((index + sizeof(boost::uint32_t)) > baseSize) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Try to read beyond the end.")
		);
	}
	this->getCopyLE(index, sizeof(boost::uint32_t), &stringSize);
	if ((index + sizeof(boost::uint32_t) + stringSize) > baseSize) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Try to read beyond the end.")
		);
	}
	result.resize(stringSize);
	std::copy(
		Base::begin() + static_cast<Base::difference_type>(index + sizeof(boost::uint32_t)),
		Base::begin() + static_cast<Base::difference_type>(index + sizeof(boost::uint32_t) + stringSize),
		result.begin()
	);
	return result;
}


template <>
const std::string OctetBlock::getBE(const Base::size_type index) const {
	const Base::size_type baseSize = Base::size();
	std::string result;
	boost::uint32_t stringSize;
	if ((index + sizeof(boost::uint32_t)) > baseSize) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Try to read beyond the end.")
		);
	}
	this->getCopyBE(index, sizeof(boost::uint32_t), &stringSize);
	if ((index + sizeof(boost::uint32_t) + stringSize) > baseSize) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Try to read beyond the end.")
		);
	}
	result.resize(stringSize);
	std::copy(
		Base::begin() + static_cast<Base::difference_type>(index + sizeof(boost::uint32_t)),
		Base::begin() + static_cast<Base::difference_type>(index + sizeof(boost::uint32_t) + stringSize),
		result.begin()
	);
	return result;
}


template <>
void OctetBlock::setLE(const Base::size_type index, const std::string & value) {
	const boost::uint32_t stringSize = static_cast<const boost::uint32_t>(value.size());
	if ((index + sizeof(boost::uint32_t) + value.size()) > Base::size()) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Try to write beyond the end.")
		);
	}
	this->setCopyLE(index, sizeof(boost::uint32_t), &stringSize);
	this->setCopyLE(index + sizeof(boost::uint32_t), stringSize, value.data());
}


void OctetBlock::setLE(const Base::size_type index, const char * value) {
	this->setLE(index, std::string(value));
}


template <>
void OctetBlock::setBE(const Base::size_type index, const std::string & value) {
	const boost::uint32_t stringSize = static_cast<const boost::uint32_t>(value.size());
	if ((index + sizeof(boost::uint32_t) + value.size()) > Base::size()) {
		BOOST_THROW_EXCEPTION(
			pcf::exception::OutOfRange()
			<< pcf::exception::tag::Message("Try to write beyond the end.")
		);
	}
	this->setCopyBE(index, sizeof(boost::uint32_t), &stringSize);
	this->setCopyLE(index + sizeof(boost::uint32_t), stringSize, value.data());
}


void OctetBlock::setBE(const Base::size_type index, const char * value) {
	this->setBE(index, std::string(value));
}


} /* namespace data */
} /* namespace pcf */
