/**
 * @file Signal.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2016 Daniel Starke
 * @date 2014-11-12
 * @version 2016-11-16
 */
#ifndef __LIBPCFXX_OS_SIGNAL_HPP__
#define __LIBPCFXX_OS_SIGNAL_HPP__


#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>


namespace pcf {
namespace os {


/**
 * This class handles signals in a background I/O service.
 * It prevents that the I/O service keeps running when only a signal handler is left.
 */
class BackgroundSignalHandler : boost::noncopyable {
public:
	/** Type of the callback function. */
	typedef boost::function1<void, int> Callback;
private:
	Callback callback; /**< Callback function which is called on signal. */
	boost::asio::io_service signalIoService; /**< Own I/O service. */
	boost::asio::signal_set signals; /**< Set of signals to handle. */
	boost::thread worker; /**< Worker thread. */
public:
	/**
	 * Constructor.
	 *
	 * @param[in] c - callback on signal
	 */
	explicit BackgroundSignalHandler(const Callback & c) :
		callback(c),
		signals(signalIoService)
	{}
	
	/**
	 * Constructor.
	 *
	 * @param[in] c - callback on signal
	 * @param[in] sig1 - add this signal
	 */
	explicit BackgroundSignalHandler(const Callback & c, const int sig1) :
		callback(c),
		signals(signalIoService)
	{
		this->signals.add(sig1);
	}
	
	/**
	 * Constructor.
	 *
	 * @param[in] c - callback on signal
	 * @param[in] sig1 - add this signal
	 * @param[in] sig2 - add this signal
	 */
	explicit BackgroundSignalHandler(const Callback & c, const int sig1, const int sig2) :
		callback(c),
		signals(signalIoService)
	{
		this->signals.add(sig1);
		this->signals.add(sig2);
	}
	
	/**
	 * Constructor.
	 *
	 * @param[in] c - callback on signal
	 * @param[in] sig1 - add this signal
	 * @param[in] sig2 - add this signal
	 * @param[in] sig3 - add this signal
	 */
	explicit BackgroundSignalHandler(const Callback & c, const int sig1, const int sig2, const int sig3) :
		callback(c),
		signals(signalIoService)
	{
		this->signals.add(sig1);
		this->signals.add(sig2);
		this->signals.add(sig3);
	}
	
	/**
	 * Destructor.
	 */
	~BackgroundSignalHandler() {
		this->signals.cancel();
		this->signalIoService.stop();
		if ( this->worker.joinable() ) this->worker.join();
	}
	
	/**
	 * Return the boost::asio::signal_set element.
	 * Use its add() and remove() functions to set signals.
	 *
	 * @return signal set
	 */
	boost::asio::signal_set & getSignalSet() {
		return this->signals;
	}
	
	/**
	 * Start the asynchronous signal handling for the defined signals.
	 */
	void asyncWaitForSignal() {
		this->signals.async_wait(boost::bind(
			&BackgroundSignalHandler::handleSignal,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::signal_number
		));
		if (this->worker.get_id() == boost::thread::id()) {
			/* thread not yet started, start now */
			this->worker = boost::thread(boost::bind(&boost::asio::io_service::run, &(this->signalIoService)));
		}
	}
	
	/**
	 * Cancel the signal handler.
	 */
	void cancel() {
		this->signals.cancel();
		this->signalIoService.stop();
		if ( this->worker.joinable() ) this->worker.join();
		this->signalIoService.reset();
	}
private:
	/**
	 * Handles the signals.
	 *
	 * @param[in] error - associated error message
	 * @param[in] signal - signal that was fired
	 */
	void handleSignal(const boost::system::error_code & error, const int signal) {
		if ( error ) return; /* do nothing on error (probably just canceled) */
		if ( this->callback ) {
			this->callback(signal);
		}
		this->asyncWaitForSignal();
	}
};


} /* namespace os */
} /* namespace pcf */


#endif /* __LIBPCFXX_OS_SIGNAL_HPP__ */
