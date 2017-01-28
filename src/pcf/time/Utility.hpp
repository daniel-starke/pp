/**
 * @file Utility.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2017 Daniel Starke
 * @date 2014-09-28
 * @version 2016-11-24
 */
#ifndef __LIBPCFXX_TIME_UTILITY_HPP__
#define __LIBPCFXX_TIME_UTILITY_HPP__

#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/thread/mutex.hpp>
#include <libpcfxx.hpp>
#include <pcf/exception/General.hpp>


namespace pcf {
namespace time {


/* Unix time */
LIBPCFXX_DLLPORT extern const boost::posix_time::ptime unixEpoch;
LIBPCFXX_DLLPORT boost::uint64_t LIBPCFXX_DECL toUnixTime(const boost::posix_time::ptime & t);
LIBPCFXX_DLLPORT boost::posix_time::ptime LIBPCFXX_DECL fromUnixTime(const boost::uint64_t t);


/* SQL time */
LIBPCFXX_DLLPORT extern const boost::posix_time::ptime sqlEpoch;
LIBPCFXX_DLLPORT boost::uint64_t LIBPCFXX_DECL toSqlTime(const boost::posix_time::ptime & t);
LIBPCFXX_DLLPORT boost::posix_time::ptime LIBPCFXX_DECL fromSqlTime(const boost::uint64_t t);


/**
 * Utility class to handle the progress state for some operation.
 *
 * @tparam BaseT - type of the progress value and total.
 * @remarks Use ProgressClock with default type boost::uint64_t.
 */
template <typename BaseT>
class BasicProgressClock {
public:
	/** Value type for the state. */
	typedef BaseT value_type;
protected:
	/**
	 * Holds a history element.
	 * 
	 * @internal
	 */
	struct HistoryElement {
		boost::posix_time::ptime dateTime; /**> date time for the specific progress */
		value_type value; /**< specific progress */
	};
	value_type current; /**< Current progress state value. */
	value_type total; /**< Final progress state value. */
	boost::optional<HistoryElement> startElement; /**< First history element every. */
	boost::circular_buffer<HistoryElement> history; /**< History of progress needed to calculate ETA. */
	mutable boost::mutex mutex; /** private mutex */
public:
	/**
	 * Constructor.
	 *
	 * @param[in] initialTotal - initial final progress state value (defaults to 0)
	 * @param[in] historySize - maximal number of elements in the progress history (defaults to the minimum of 3)
	 */
	explicit BasicProgressClock(const value_type initialTotal = 0, const size_t historySize = 3):
		history(historySize)
	{
		if (historySize < 3) {
			/* minimal history size is 3 */
			BOOST_THROW_EXCEPTION(
				pcf::exception::InvalidValue()
				<< pcf::exception::tag::Message(std::string("A history size of ")
					+ boost::lexical_cast<std::string>(historySize)
					+ " is invalid. Set this value at least to 3.")
			);
		}
		this->reset();
		this->total = initialTotal;
	}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	BasicProgressClock(const BasicProgressClock & o):
		current(o.current),
		total(o.total),
		startElement(o.startElement),
		history(o.history)
	{}
	
	/**
	 * Assign operator.
	 *
	 * @param[in] o - object to assign
	 * @return reference to this object
	 */
	BasicProgressClock & operator= (const BasicProgressClock & o) {
		if (this != &o) {
			this->current = o.current;
			this->total = o.total;
			this->startElement = o.startElement;
			this->history = o.history;
		}
		return *this;
	}
	
	/**
	 * Resets the progress state and final progress state value.
	 * This also clears the history and resets the first history element.
	 *
	 * @return referent to own instance for chained operations
	 */
	BasicProgressClock & reset() {
		boost::mutex::scoped_lock lock(this->mutex);
		this->current = 0;
		this->total = 0;
		this->startElement = boost::optional<HistoryElement>();
		this->history.clear();
		return *this;
	}
	
	/**
	 * Adds a given amount to the final progress state value.
	 *
	 * @param[in] val - add this amount
	 * @return referent to own instance for chained operations
	 */
	BasicProgressClock & addTotal(const value_type val) {
		boost::mutex::scoped_lock lock(this->mutex);
		this->total += val;
		return *this;
	}
	
	/**
	 * Sets the starting value and date time of now (UTC).
	 *
	 * @param[in] val - set to this value
	 * @return referent to own instance for chained operations
	 */
	BasicProgressClock & start(const value_type val) {
		boost::mutex::scoped_lock lock(this->mutex);
		HistoryElement histElem;
		histElem.dateTime = boost::posix_time::microsec_clock::universal_time();
		histElem.value = val;
		this->startElement.reset(histElem);
		this->current = val;
		return *this;
	}
	
	/**
	 * Sets the starting value and passed date time.
	 *
	 * @param[in] val - set to this value
	 * @param[in] dt - set to this date time
	 * @return referent to own instance for chained operations
	 */
	BasicProgressClock & start(const value_type val, const boost::posix_time::ptime & dt) {
		boost::mutex::scoped_lock lock(this->mutex);
		HistoryElement histElem;
		histElem.dateTime = dt;
		histElem.value = val;
		this->startElement.reset(histElem);
		this->current = val;
		return *this;
	}
	
	/**
	 * Adds a given amount to the current progress state value.
	 * Assumes that the amount was added at the same time this function was called (UTC).
	 *
	 * @param[in] val - add this amount
	 * @return referent to own instance for chained operations
	 */
	BasicProgressClock & add(const value_type val) {
		boost::mutex::scoped_lock lock(this->mutex);
		HistoryElement histElem;
		histElem.dateTime = boost::posix_time::microsec_clock::universal_time();
		histElem.value = val;
		if ( ! this->startElement ) this->startElement.reset(histElem);
		this->history.push_back(histElem);
		this->current += val;
		return *this;
	}
	
	/**
	 * Adds a given amount to the current progress state value.
	 *
	 * @param[in] val - add this amount
	 * @param[in] dt - the amount was added at this date time
	 * @return referent to own instance for chained operations
	 */
	BasicProgressClock & add(const value_type val, const boost::posix_time::ptime & dt) {
		boost::mutex::scoped_lock lock(this->mutex);
		HistoryElement histElem;
		histElem.dateTime = dt;
		histElem.value = val;
		if ( ! this->startElement ) this->startElement.reset(histElem);
		this->history.push_back(histElem);
		this->current += val;
		return *this;
	}
	
	/**
	 * Returns the current progress state value.
	 *
	 * @return current progress state value
	 */
	value_type getCurrent() const {
		return this->current;
	}
	
	/**
	 * Returns the final progress state value.
	 *
	 * @return final progress state value
	 */
	value_type getTotal() const {
		return this->total;
	}
	
	/**
	 * Returns the maximum size of elements in the history.
	 *
	 * @return maximal history size
	 */
	size_t getHistorySize() const {
		return static_cast<size_t>(this->history.capacity());
	}
	
	/**
	 * Returns the average speed since start.
	 *
	 * @return average speed per second
	 */
	boost::optional<double> getAvgSpeed() const {
		boost::mutex::scoped_lock lock(this->mutex);
		boost::optional<double> result;
		/* check if speed calculation is possible */
		if (this->total == 0 || this->current == 0 || ( ! this->startElement )) return result;
		if (this->current == this->total) {
			/* reached maximum */
			result.reset(0.0);
			return result;
		}
		if ( this->history.empty() ) {
			/* need infinite time to finish */
			result.reset(std::numeric_limits<double>::infinity());
			return result;
		}
		/* calculate speed for global average */
		const boost::posix_time::time_duration progressTime(this->history.back().dateTime - this->startElement->dateTime);
		result.reset(			
			static_cast<double>(this->current) / (static_cast<double>(progressTime.total_milliseconds()) / 1000.0)
		);
		return result;
	}
	
	/**
	 * Returns the average speed based on the history.
	 *
	 * @return recent average speed per second
	 */
	boost::optional<double> getRecentAvgSpeed() const {
		boost::mutex::scoped_lock lock(this->mutex);
		boost::optional<double> result;
		boost::posix_time::time_duration recentProgressTime;
		boost::posix_time::ptime lastDateTime;
		value_type recentProgressValue;
		bool firstElement;
		/* check if speed calculation is possible */
		if (this->total == 0 || this->current == 0 || ( ! this->startElement )) return result;
		if (this->current == this->total) {
			/* reached maximum */
			result.reset(0.0);
			return result;
		}
		/* calculate speed based on history */
		recentProgressTime = boost::posix_time::time_duration(0, 0, 0, 0);
		recentProgressValue = 0;
		if (this->history.size() > 1) {
			firstElement = true;
		} else {
			lastDateTime = this->startElement->dateTime;
			firstElement = false;
		}
		BOOST_FOREACH(const HistoryElement & histElem, this->history) {
			if ( firstElement ) {
				firstElement = false;
			} else {
				recentProgressTime += (histElem.dateTime - lastDateTime);
				recentProgressValue += histElem.value;
			}
			lastDateTime = histElem.dateTime;
		}
		if (recentProgressTime <= boost::posix_time::time_duration(0, 0, 0, 0) || recentProgressValue <= 0 || this->history.empty()) {
			/* need infinite time to finish */
			result.reset(std::numeric_limits<double>::infinity());
			return result;
		}
		result.reset(			
			static_cast<double>(recentProgressValue) / (static_cast<double>(recentProgressTime.total_milliseconds()) / 1000.0)
		);
		return result;
	}
	
	/**
	 * Returns the estimated time of arrival (ETA) which indicates
	 * when the progress is estimated to reach the final progress state value.
	 * The method returns no value if the requirements for this operation are not given.
	 *
	 * @return optional ETA until current == total
	 */
	boost::optional<boost::posix_time::time_duration> getEta(boost::optional<double> avgSpeed = boost::optional<double>()) const {
		boost::mutex::scoped_lock lock(this->mutex);
		boost::optional<boost::posix_time::time_duration> result;
		/* check if ETA calculation is possible */
		if (this->total == 0 || this->current == 0) return result;
		if (this->current == this->total) {
			/* reached maximum */
			result.reset(boost::posix_time::time_duration(0, 0, 0, 0));
			return result;
		}
		lock.unlock();
		if ( ! avgSpeed ) avgSpeed = this->getRecentAvgSpeed();
		lock.lock();
		if (this->history.empty() || (avgSpeed && *avgSpeed == std::numeric_limits<double>::infinity())) {
			/* need infinite time to finish */
			result.reset(boost::posix_time::time_duration(boost::posix_time::pos_infin));
			return result;
		}
		if ( ! avgSpeed ) return result;
		result.reset(boost::posix_time::milliseconds(static_cast<long>(
			(1000.0 * static_cast<double>(this->total - this->current) / (*avgSpeed)) + 0.5
		)));
		return result;
	}
	
	/**
	 * Formats the output string by the given format string in printf() manner.@n
	 * The following options are available:@n
	 * Replacement prefixes:
	 * @li %d - last update date
	 * @li %l - last update time stamp
	 * @li %r - recent average
	 * @li %g - global average
	 * 
	 * Direct replacements:
	 * @li %% - %
	 * @li %c - current state value
	 * @li %C - current state value formatted as human readable SI binary size
	 * @li %t - final state value
	 * @li %T - final state value formatted as human readable SI binary size
	 * @li %p - percentage with leading spaces
	 * @li %P - percentage without leading spaces
	 * @li %q - percentage with leading spaces and once decimal comma place
	 * @li %Q - percentage without leading spaces and once decimal comma place
	 * 
	 * Last update date suffixes:
	 * @li %dy - year part
	 * @li %dY - year part with 4 digits
	 * @li %dm - month part
	 * @li %dM - month part with 2 digits
	 * @li %dd - day of month part
	 * @li %dD - day of month part with 2 digits
	 * 
	 * Last update time stamp suffixes:
	 * @li %lh - hours part
	 * @li %lH - hours part with 2 digits
	 * @li %lm - minutes part
	 * @li %lM - minutes part with 2 digits
	 * @li %ls - seconds part
	 * @li %lS - seconds part with 2 digits
	 * @li %lf - milliseconds part
	 * @li %lF - milliseconds part with 3 digits
	 * 
	 * Recent and global suffixes ('x' stands for 'r' or 'g'):
	 * @li %xf - milliseconds part
	 * @li %xF - total number of milliseconds (rounded down)
	 * @li %xs - seconds part
	 * @li %xS - total number of seconds (rounded down)
	 * @li %xm - minutes part
	 * @li %xM - total number of minutes (rounded down)
	 * @li %xh - hours part
	 * @li %xH - total number of hours (rounded down)
	 * @li %xd - days part (rounded down)
	 * @li %xe - with two units precision
	 * @li %xE - as %d %h:%m:%s.%f whereas the %d part is omitted if zero
	 * @li %xa - average speed with dynamic time unit
	 * @li %xA - average speed formatted as human readable SI binary size
	 *
	 * @param[in] fmt - format string
	 * @param[in] unitStr - string used for an unit value
	 * @param[in] unitsStr - string used for an unit value not equal to 1
	 * @return formatted string
	 */
	template <typename CharT>
	std::basic_string<CharT> format(const std::basic_string<CharT> & fmt, const std::basic_string<CharT> & unitStr, const std::basic_string<CharT> & unitsStr) const {
		boost::mutex::scoped_lock lock(this->mutex);
		const std::basic_string<CharT> currentSize = this->getSizeString<CharT>(this->current);
		const std::basic_string<CharT> totalSize = this->getSizeString<CharT>(this->total);
		lock.unlock();
		std::basic_ostringstream<CharT> sout;
		bool withinFormatString = false;
		boost::optional<double> avgSpeed;
		const boost::optional<double> recentAvgSpeed = this->getRecentAvgSpeed();
		const boost::optional<double> globalAvgSpeed = this->getAvgSpeed();
		std::basic_string<CharT> avgSpeedStr, avgSpeedSize, recentAvgSpeedStr, recentAvgSpeedSize, globalAvgSpeedStr, globalAvgSpeedSize;
		boost::optional<boost::posix_time::time_duration> eta;
		const boost::optional<boost::posix_time::time_duration> recentEta(this->getEta(recentAvgSpeed));
		const boost::optional<boost::posix_time::time_duration> globalEta(this->getEta(globalAvgSpeed));
		boost::posix_time::ptime lastDateTime;
		enum {
			NONE,
			LAST_UPDATE_DATE,
			LAST_UPDATE_TIME,
			RECENT_AVG,
			GLOBAL_AVG
		} next = NONE;
		if ( recentAvgSpeed ) {
			if ((*recentAvgSpeed) == std::numeric_limits<double>::infinity()) {
				const std::string infStr("inf");
				recentAvgSpeedStr.assign(infStr.begin(), infStr.end());
				recentAvgSpeedSize.assign(infStr.begin(), infStr.end());
			} else {
				const std::string perSecStr("/s");
				const std::string timeUnit[4] = {"s", "min", "h", "d"};
				size_t avgSpeedTimeUnit = 0;
				double avgSpeedValue = *recentAvgSpeed;
				std::ostringstream tmpOut;
				recentAvgSpeedSize = this->getSizeString<CharT>(avgSpeedValue)
					+ std::basic_string<CharT>(perSecStr.begin(), perSecStr.end());
				if (avgSpeedValue < 1.0) {
					avgSpeedValue *= 60;
					avgSpeedTimeUnit++;
				}
				if (avgSpeedValue < 1.0) {
					avgSpeedValue *= 60;
					avgSpeedTimeUnit++;
				}
				if (avgSpeedValue < 1.0) {
					avgSpeedValue *= 24;
					avgSpeedTimeUnit++;
				}
				double rint;
				const double frac = modf(avgSpeedValue, &rint);
				tmpOut.precision(0);
				tmpOut << std::fixed << avgSpeedValue << static_cast<CharT>(' ');
				if ((frac < 0.5 && rint == 1.0) || (frac >= 0.5 && rint == 0.0)) {
					tmpOut << unitStr;
				} else {
					tmpOut << unitsStr;
				}
				tmpOut << static_cast<CharT>('/') << timeUnit[avgSpeedTimeUnit];
				recentAvgSpeedStr = tmpOut.str();
			}
		} else {
			const std::string naStr("n/a");
			recentAvgSpeedStr.assign(naStr.begin(), naStr.end());
			recentAvgSpeedSize.assign(naStr.begin(), naStr.end());
		}
		if ( globalAvgSpeed ) {
			if ((*globalAvgSpeed) == std::numeric_limits<double>::infinity()) {
				const std::string infStr("inf");
				globalAvgSpeedStr.assign(infStr.begin(), infStr.end());
				globalAvgSpeedSize.assign(infStr.begin(), infStr.end());
			} else {
				const std::string perSecStr("/s");
				const std::string timeUnit[4] = {"s", "min", "h", "d"};
				size_t avgSpeedTimeUnit = 0;
				double avgSpeedValue = *globalAvgSpeed;
				std::ostringstream tmpOut;
				globalAvgSpeedSize = this->getSizeString<CharT>(avgSpeedValue)
					+ std::basic_string<CharT>(perSecStr.begin(), perSecStr.end());
				if (avgSpeedValue < 1.0) {
					avgSpeedValue *= 60;
					avgSpeedTimeUnit++;
				}
				if (avgSpeedValue < 1.0) {
					avgSpeedValue *= 60;
					avgSpeedTimeUnit++;
				}
				if (avgSpeedValue < 1.0) {
					avgSpeedValue *= 24;
					avgSpeedTimeUnit++;
				}
				double rint;
				const double frac = modf(avgSpeedValue, &rint);
				tmpOut.precision(0);
				tmpOut << std::fixed << avgSpeedValue << static_cast<CharT>(' ');
				if ((frac < 0.5 && rint == 1.0) || (frac >= 0.5 && rint == 0.0)) {
					tmpOut << unitStr;
				} else {
					tmpOut << unitsStr;
				}
				tmpOut << static_cast<CharT>('/') << timeUnit[avgSpeedTimeUnit];
				globalAvgSpeedStr = tmpOut.str();
			}
		} else {
			const std::string naStr("n/a");
			globalAvgSpeedStr.assign(naStr.begin(), naStr.end());
			globalAvgSpeedSize.assign(naStr.begin(), naStr.end());
		}
		lock.lock();
		if ( this->startElement ) lastDateTime = this->startElement->dateTime;
		if ( ! this->history.empty() ) lastDateTime = this->history.back().dateTime;
		BOOST_FOREACH(const CharT c, fmt) {
			if ( ! withinFormatString ) {
				if (c == '%') {
					withinFormatString = true;
					next = NONE;
				} else {
					sout << c;
				}
			} else {
				withinFormatString = false;
				switch (next) {
				case NONE:
					switch (c) {
					/* escape % */
					case '%': sout << static_cast<CharT>('%'); break;
					/* prefix style */
					case 'd': withinFormatString = true; next = LAST_UPDATE_DATE; break;
					case 'l': withinFormatString = true; next = LAST_UPDATE_TIME; break;
					case 'r':
						withinFormatString = true;
						next = RECENT_AVG;
						avgSpeedStr = recentAvgSpeedStr;
						avgSpeedSize = recentAvgSpeedSize;
						eta = recentEta;
						break;
					case 'g':
						withinFormatString = true;
						next = GLOBAL_AVG;
						avgSpeedStr = globalAvgSpeedStr;
						avgSpeedSize = globalAvgSpeedSize;
						eta = globalEta;
						break;
					/* direct replacements */
					case 'c': sout << this->current; break;
					case 'C': sout << currentSize; break;
					case 't': sout << this->total; break;
					case 'T': sout << totalSize; break;
					case 'p':
					case 'q':
						if (this->total <= 0) {
							sout << 100;
						} else {
							sout.precision(c == 'p' ? 0 : 1);
							sout
								<< std::setfill(static_cast<CharT>(' '))
								<< std::setw(c == 'p' ? 3 : 5)
								<< std::fixed
								<< (100.0 * static_cast<double>(this->current) / static_cast<double>(this->total));
						}
						break;
					case 'P':
					case 'Q':
						if (this->total <= 0) {
							sout << 100;
						} else {
							sout.precision(c == 'P' ? 0 : 1);
							sout
								<< std::fixed
								<< (100.0 * static_cast<double>(this->current) / static_cast<double>(this->total));
						}
						break;
					default:
						BOOST_THROW_EXCEPTION(
							pcf::exception::InvalidValue()
							<< pcf::exception::tag::Message(std::string("Invalid format tag value '%") + static_cast<char>(c) + "'.")
						);
						break;
					}
					break;
				case LAST_UPDATE_DATE:
					switch (c) {
					case 'y':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?');
						} else {
							sout << static_cast<int>(lastDateTime.date().year());
						}
						break;
					case 'Y':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?') << static_cast<CharT>('?') << static_cast<CharT>('?') << static_cast<CharT>('?');
						} else {
							sout << std::setfill(static_cast<CharT>('0')) << std::setw(4) << static_cast<int>(lastDateTime.date().year());
						}
						break;
					case 'm':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?');
						} else {
							sout << static_cast<int>(lastDateTime.date().month());
						}
						break;
					case 'M':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?') << static_cast<CharT>('?');
						} else {
							sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << static_cast<int>(lastDateTime.date().month());
						}
						break;
					case 'd':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?');
						} else {
							sout << static_cast<int>(lastDateTime.date().day());
						}
						break;
					case 'D':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?') << static_cast<CharT>('?');
						} else {
							sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << static_cast<int>(lastDateTime.date().day());
						}
						break;
					default:
						BOOST_THROW_EXCEPTION(
							pcf::exception::InvalidValue()
							<< pcf::exception::tag::Message(std::string("Invalid format tag value '%d") + static_cast<char>(c) + "'.")
						);
						break;
					}
					next = NONE;
					break;
				case LAST_UPDATE_TIME:
					switch (c) {
					case 'h':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?');
						} else {
							sout << static_cast<int>(lastDateTime.time_of_day().hours());
						}
						break;
					case 'H':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?') << static_cast<CharT>('?');
						} else {
							sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << static_cast<int>(lastDateTime.time_of_day().hours());
						}
						break;
					case 'm':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?');
						} else {
							sout << static_cast<int>(lastDateTime.time_of_day().minutes());
						}
						break;
					case 'M':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?') << static_cast<CharT>('?');
						} else {
							sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << static_cast<int>(lastDateTime.time_of_day().minutes());
						}
						break;
					case 's':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?');
						} else {
							sout << static_cast<int>(lastDateTime.time_of_day().seconds());
						}
						break;
					case 'S':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?') << static_cast<CharT>('?');
						} else {
							sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << static_cast<int>(lastDateTime.time_of_day().seconds());
						}
						break;
					case 'f':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?');
						} else {
							sout << static_cast<int>(lastDateTime.time_of_day().total_milliseconds() % 1000);
						}
						break;
					case 'F':
						if ( lastDateTime.is_special() ) {
							sout << static_cast<CharT>('?') << static_cast<CharT>('?');
						} else {
							sout << std::setfill(static_cast<CharT>('0')) << std::setw(3) << static_cast<int>(lastDateTime.time_of_day().total_milliseconds() % 1000);
						}
						break;
					default:
						BOOST_THROW_EXCEPTION(
							pcf::exception::InvalidValue()
							<< pcf::exception::tag::Message(std::string("Invalid format tag value '%t") + static_cast<char>(c) + "'.")
						);
						break;
					}
					next = NONE;
					break;
				case RECENT_AVG:
				case GLOBAL_AVG:
					switch (c) {
					case 'f': if ( eta ) sout << std::setfill(static_cast<CharT>('0')) << std::setw(3) << (eta->total_milliseconds() % 1000); break;
					case 'F': if ( eta ) sout << eta->total_milliseconds(); break;
					case 's': if ( eta ) sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << eta->seconds(); break;
					case 'S': if ( eta ) sout << eta->total_seconds(); break;
					case 'm': if ( eta ) sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << eta->minutes(); break;
					case 'M': if ( eta ) sout << (eta->total_seconds() / 60); break;
					case 'h': if ( eta ) sout << std::setfill(static_cast<CharT>('0')) << std::setw(2) << (eta->hours() % 24); break;
					case 'H': if ( eta ) sout << (eta->total_seconds() / 3600); break;
					case 'd': if ( eta ) sout << (eta->hours() / 24); break;
					case 'e':
						if ( eta ) {
							if ( eta->is_pos_infinity() ) {
								sout << static_cast<CharT>('i') << static_cast<CharT>('n') << static_cast<CharT>('f');
							} else if (eta->hours() >= 24) {
								sout
									<< (eta->hours() / 24) << static_cast<CharT>('d') << static_cast<CharT>(' ')
									<< (eta->hours() % 24)  << static_cast<CharT>('h');
							} else if (eta->hours() > 0) {
								sout
									<< (eta->hours() % 24) << static_cast<CharT>('h') << static_cast<CharT>(' ')
									<< eta->minutes() << static_cast<CharT>('m');
							} else if (eta->minutes() > 0) {
								sout
									<< eta->minutes() << static_cast<CharT>('m') << static_cast<CharT>(' ')
									<< eta->seconds() << static_cast<CharT>('s');
							} else {
								sout
									<< eta->seconds() << static_cast<CharT>('s');
							}
						} else {
							sout << static_cast<CharT>('n') << static_cast<CharT>('/') << static_cast<CharT>('a');
						}
						break;
					case 'E':
						if ( eta ) {
							if ( eta->is_pos_infinity() ) {
								sout << static_cast<CharT>('i') << static_cast<CharT>('n') << static_cast<CharT>('f');
							} else {
								if (eta->hours() >= 24) {
									sout << (eta->hours() / 24) << static_cast<CharT>(' ');
								}
								sout
									<< std::setfill(static_cast<CharT>('0')) << std::setw(2) << (eta->hours() % 24) << static_cast<CharT>(':')
									<< std::setfill(static_cast<CharT>('0')) << std::setw(2) << eta->minutes() << static_cast<CharT>(':')
									<< std::setfill(static_cast<CharT>('0')) << std::setw(2) << eta->seconds() << static_cast<CharT>('.')
									<< std::setfill(static_cast<CharT>('0')) << std::setw(3) << (eta->total_milliseconds() % 1000);
							}
						} else {
							sout << static_cast<CharT>('n') << static_cast<CharT>('/') << static_cast<CharT>('a');
						}
						break;
					case 'a': sout << avgSpeedStr; break;
					case 'A': sout << avgSpeedSize; break;
					default:
						if (next == RECENT_AVG) {
							BOOST_THROW_EXCEPTION(
								pcf::exception::InvalidValue()
								<< pcf::exception::tag::Message(std::string("Invalid format tag value '%r") + static_cast<char>(c) + "'.")
							);
						} else {
							BOOST_THROW_EXCEPTION(
								pcf::exception::InvalidValue()
								<< pcf::exception::tag::Message(std::string("Invalid format tag value '%g") + static_cast<char>(c) + "'.")
							);
						}
						break;
					}
					next = NONE;
					break;
				}
			}
		}
		return sout.str();
	}
	
	/**
	 * Formats the output string by the given format string in printf() manner.
	 * Unit is set to "element".
	 * 
	 * @param[in] fmt - format string
	 * @param[in] unitStr - string used for an unit value
	 * @param[in] unitsStr - string used for an unit value not equal to 1
	 * @return formatted string
	 * @see format(const std::basic_string<CharT> &, const std::basic_string<CharT> &, const std::basic_string<CharT> &) for format options
	 */
	template <typename CharT>
	std::basic_string<CharT> format(const CharT * fmt, const CharT * unitStr, const CharT * unitsStr) const {
		const std::basic_string<CharT> unit(unitStr);
		const std::basic_string<CharT> units(unitsStr);
		return this->format(std::basic_string<CharT>(fmt), std::basic_string<CharT>(unit.begin(), unit.end()), std::basic_string<CharT>(units.begin(), units.end()));
	}
	
	/**
	 * Formats the output string by the given format string in printf() manner.
	 * Unit is set to "element".
	 * 
	 * @param[in] fmt - format string
	 * @return formatted string
	 * @see format(const std::basic_string<CharT> &, const std::basic_string<CharT> &, const std::basic_string<CharT> &) for format options
	 */
	template <typename CharT>
	std::basic_string<CharT> format(const std::basic_string<CharT> & fmt) const {
		const std::string unit("element");
		const std::string units("elements");
		return this->format(fmt, std::basic_string<CharT>(unit.begin(), unit.end()), std::basic_string<CharT>(units.begin(), units.end()));
	}
	
	/**
	 * Formats the output string by the given format string in printf() manner.
	 * Unit is set to "element".
	 * 
	 * @param[in] fmt - format string
	 * @return formatted string
	 * @see format(const std::basic_string<CharT> &, const std::basic_string<CharT> &, const std::basic_string<CharT> &) for format options
	 */
	template <typename CharT>
	std::basic_string<CharT> format(const CharT * fmt) const {
		const std::string unit("element");
		const std::string units("elements");
		return this->format(std::basic_string<CharT>(fmt), std::basic_string<CharT>(unit.begin(), unit.end()), std::basic_string<CharT>(units.begin(), units.end()));
	}
protected:
	/**
	 * Returns the given size reduced to the best fitting binary SI unit.
	 *
	 * @param[in] size - size to fit
	 * @return string with the fitted size and unit
	 */
	template <typename CharT, typename T>
	static std::basic_string<CharT> getSizeString(const T size) {
		static const std::vector<std::string> units = boost::assign::list_of<std::string>
			("bytes")
			("KiB")
			("MiB")
			("GiB")
			("TiB")
			("PiB")
			("EiB")
			("ZiB")
			("YiB")
		;
		double val = static_cast<double>(size);
		size_t i;
		std::basic_ostringstream<CharT> sout;
		for (i = 0; (i + 1) < units.size() && val > 1024.0; i++, val /= 1024.0);
		sout.precision(i == 0 ? 0 : 1);
		const std::basic_string<CharT> unitStr(units[i].begin(), units[i].end());
		sout << std::fixed << val << static_cast<CharT>(' ');
		double rint;
		const double frac = modf(val, &rint);
		if (i == 0 && ((frac < 0.5 && rint == 1.0) || (frac >= 0.5 && rint == 0.0))) {
			const std::string byteStr("byte");
			sout << std::basic_string<CharT>(byteStr.begin(), byteStr.end());
		} else {
			sout << unitStr;
		}
		return sout.str();
	}
};


/** Default progress clock using boost::uint64_t. @see BasicProgressClock */
typedef BasicProgressClock<boost::uint64_t> ProgressClock;


} /* namespace time */
} /* namespace pcf */


#endif /* __LIBPCFXX_TIME_UTILITY_HPP__ */
