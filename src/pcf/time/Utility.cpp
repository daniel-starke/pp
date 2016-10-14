/**
 * @file Utility.cpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-09-28
 * @version 2016-05-01
 */
#include <boost/date_time/gregorian/gregorian.hpp>
#include <pcf/time/Utility.hpp>


namespace pcf {
namespace time {


/** Unix time epoch */
const boost::posix_time::ptime unixEpoch = boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));


/**
 * Converts the passed Posix time to a Unix time value.
 *
 * @param[in] t - Posix time to convert
 * @return Unix time representation
 */
boost::uint64_t toUnixTime(const boost::posix_time::ptime & t) {
	const boost::posix_time::time_duration dur(t - unixEpoch);
	return static_cast<boost::uint64_t>(dur.ticks() / dur.ticks_per_second());
}


/**
 * Converts the passed Unix time to a Posix time value.
 *
 * @param[in] t - Unix time to convert
 * @return Posix time representation
 */
boost::posix_time::ptime fromUnixTime(const boost::uint64_t t) {
	return boost::posix_time::ptime(unixEpoch)
		+ boost::posix_time::time_duration(
			0,
			0,
			0,
			boost::posix_time::time_duration::ticks_per_second() * static_cast<boost::posix_time::hours::fractional_seconds_type>(t)
		);
}


/** SQL time epoch */
const boost::posix_time::ptime sqlEpoch = boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1));


/**
 * Converts the passed Posix time to a SQL time value.
 *
 * @param[in] t - Posix time to convert
 * @return SQL time representation
 */
boost::uint64_t toSqlTime(const boost::posix_time::ptime & t) {
	const boost::posix_time::time_duration dur(t - sqlEpoch);
	return static_cast<boost::uint64_t>(dur.ticks() / dur.ticks_per_second());
}


/**
 * Converts the passed SQL time to a Posix time value.
 *
 * @param[in] t - SQL time to convert
 * @return Posix time representation
 */
boost::posix_time::ptime fromSqlTime(const boost::uint64_t t) {
	return boost::posix_time::ptime(sqlEpoch)
		+ boost::posix_time::time_duration(
			0,
			0,
			0,
			boost::posix_time::time_duration::ticks_per_second() * static_cast<boost::posix_time::hours::fractional_seconds_type>(t)
		);
}


} /* namespace time */
} /* namespace pcf */
