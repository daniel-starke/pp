/**
 * @file Utility.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @date 2015-08-04
 * @version 2016-11-20
 */
#ifndef __LIBPCFXX_PROCESS_UTILITY_HPP__
#define __LIBPCFXX_PROCESS_UTILITY_HPP__


#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <boost/atomic.hpp>
#include <boost/foreach.hpp>
#include <pcf/exception/General.hpp>
#include <pcf/os/Target.hpp>
#include <libpcfxx.hpp>


extern "C" {
#ifdef PCF_IS_WIN
#include <libpcf/fdious.h>
#else /* ! PCF_IS_WIN */
#include <libpcf/fdios.h>
#endif /* PCF_IS_WIN */
}


namespace pcf {
namespace process {


tFdioPMode operator| (const tFdioPMode a, const tFdioPMode b);


/**
 * Helper class to manage a single process handle.
 */
class ProcessPipe {
public:
#ifdef PCF_IS_WIN
	typedef wchar_t char_type;
	typedef HANDLE pid_type;
#else /* ! PCF_IS_WIN */
	typedef char char_type;
	typedef pid_t pid_type;
#endif /* PCF_IS_WIN */
	typedef std::basic_string<char_type> string_type;
private:
	/** Defines the internal process/pipe handle type. */
	typedef tFdioPHandle HandleType;
	HandleType * handle; /**< Used process/pipe handle. */
	volatile mutable boost::atomic<size_t> * references; /**< Used for reference counting. */
public:
	/** 
	 * Constructor.
	 *
	 * @param[in,out] h - process/pipe handle
	 */
	explicit ProcessPipe(HandleType * h):
		handle(h),
		references(new boost::atomic<size_t>(0))
	{
		this->addReference();
	}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	ProcessPipe(const ProcessPipe & o):
		handle(o.handle),
		references(o.references)
	{
		this->addReference();
	}
	
	/**
	 * Destructor.
	 */
	~ProcessPipe() {
		this->releaseReference();
	}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 * @return new object
	 */
	ProcessPipe & operator= (const ProcessPipe & o) {
		if (this != &o) {
			this->handle = o.handle;
			this->references = o.references;
			this->addReference();
		}
		return *this;
	}
	
	/**
	 * Return the file descriptor to the standard input of the running
	 * process.
	 *
	 * @return file descriptor for standard input or NULL if not associated
	 * @throws pcf::exception::NullPointer if the internal handle is NULL
	 */
	FILE * getIn() {
		if (this->handle == NULL) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::NullPointer()
				<< pcf::exception::tag::Message("Invalid process handle.")
			);
		}
		return this->handle->in;
	}
	
	/**
	 * Return the numeric file descriptor to the standard input of the running
	 * process.
	 *
	 * @return numeric file descriptor for standard input or -1 if not associated
	 * @throws pcf::exception::NullPointer if the internal handle is NULL
	 */
	int getInFd() {
#ifdef PCF_IS_WIN
		return _fileno(this->getIn());
#else /* ! PCF_IS_WIN */
		return fileno(this->getIn());
#endif /* PCF_IS_WIN */
	}
	
	/**
	 * Return the file descriptor to the standard output of the running
	 * process.
	 *
	 * @return file descriptor for standard output or NULL if not associated
	 * @throws pcf::exception::NullPointer if the internal handle is NULL
	 */
	FILE * getOut() {
		if (this->handle == NULL) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::NullPointer()
				<< pcf::exception::tag::Message("Invalid process handle.")
			);
		}
		return this->handle->out;
	}
	
	/**
	 * Return the numeric file descriptor to the standard output of the running
	 * process.
	 *
	 * @return numeric file descriptor for standard output or -1 if not associated
	 * @throws pcf::exception::NullPointer if the internal handle is NULL
	 */
	int getOutFd() {
#ifdef PCF_IS_WIN
		return _fileno(this->getOut());
#else /* ! PCF_IS_WIN */
		return fileno(this->getOut());
#endif /* PCF_IS_WIN */
	}
	
	/**
	 * Return the file descriptor to the standard error of the running
	 * process.
	 *
	 * @return file descriptor for standard error or NULL if not associated
	 * @throws pcf::exception::NullPointer if the internal handle is NULL
	 */
	FILE * getErr() {
		if (this->handle == NULL) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::NullPointer()
				<< pcf::exception::tag::Message("Invalid process handle.")
			);
		}
		return this->handle->err;
	}
	
	/**
	 * Return the numeric file descriptor to the standard error of the running
	 * process.
	 *
	 * @return numeric file descriptor for standard error or -1 if not associated
	 * @throws pcf::exception::NullPointer if the internal handle is NULL
	 */
	int getErrFd() {
#ifdef PCF_IS_WIN
		return _fileno(this->getErr());
#else /* ! PCF_IS_WIN */
		return fileno(this->getErr());
#endif /* PCF_IS_WIN */
	}
	
	/**
	 * Returns the implementation specific process handle to the open process.
	 * 
	 * @return process handle
	 * @throws pcf::exception::NullPointer if the internal handle is NULL
	 */
	pid_type getPid() {
		if (this->handle == NULL) {
			BOOST_THROW_EXCEPTION(
				pcf::exception::NullPointer()
				<< pcf::exception::tag::Message("Invalid process handle.")
			);
		}
		return this->handle->pid;
	}
	
	/** 
	 * Waits until the open process has finished and returns its exit code.
	 * 
	 * @return exit code or -1 if the internal handle is NULL or wait() was called more than once
	 */
	int wait() {
		if (this->handle != NULL) {
			int result = 0;
#ifdef PCF_IS_WIN
			result = fdious_pclose(this->handle);
#else /* ! PCF_IS_WIN */
			result = fdios_pclose(this->handle);
#endif /* PCF_IS_WIN */
			this->handle = NULL;
			return result;
		}
		return -1;
	}
	
	/**
	 * Returns whether the internal handle is not NULL.
	 *
	 * @return true if not NULL, else false
	 */
	operator bool() const {
		return this->handle != NULL;
	}
private:
	/**
	 * Adds one to the internal reference counter.
	 */
	inline void addReference() const {
		this->references->fetch_add(1, boost::memory_order_relaxed);
	}
	
	/**
	 * Releases one from the internal reference counter.
	 * The handle is closed if this was the last reference.
	 */
	inline void releaseReference() {
		if (this->references->fetch_sub(1, boost::memory_order_release) == 1) {
			if (this->handle != NULL) {
#ifdef PCF_IS_WIN
				fdious_pclose(this->handle);
#else /* ! PCF_IS_WIN */
				fdios_pclose(this->handle);
#endif /* PCF_IS_WIN */
				this->handle = NULL;
			}
			delete this->references;
		}
	}
};


/**
 * Creates a process/pipe generator based on the given command-line arguments.
 */
class ProcessPipeFactory {
public:
	/** Character type for the argument values. */
	typedef ProcessPipe::char_type char_type;
	/** String type for the argument values. */
	typedef ProcessPipe::string_type string_type;
	/** Command-line type namespace. */
	struct Type {
		/** Command-line shell type. (see man sh) */
		static const struct ShellTag {} Shell;
		/** Raw command-line shell type. (only useful in Windows) */
		static const struct RawTag {} Raw;
	};
private:
	/** Binary file path. */
	string_type binaryPath;
	/** NULL terminated argument array. */
	char_type ** args;
	/** Total number of bytes used to store args. */
	size_t argsSize;
	/** Flag whether to run in raw mode or not. */
	bool raw;
public:
	/**
	 * Constructor.
	 *
	 * @param[in] binPath - path to the binary which shall be executed
	 * @param[in] procArgs - command-line arguments
	 */
	explicit ProcessPipeFactory(const string_type & binPath, const std::vector<string_type> & procArgs);
	
	/**
	 * Constructor.
	 * Example value: /bin/sh -c "echo \"Hello World!\""
	 *
	 * @param[in] binPath - path to the binary which shall be executed
	 * @param[in] cmdLine - command-line
	 * @param[in] sig - type signature
	 * @throws pcf::exception::SyntaxError on syntax error of the command-line
	 */
	explicit ProcessPipeFactory(const string_type & binPath, const string_type & cmdLine, const Type::ShellTag & sig);
	
	/**
	 * Constructor.
	 * Example value: cmd.exe /c echo "Hello World!"
	 *
	 * @param[in] binPath - path to the binary which shall be executed
	 * @param[in] cmdLine - command-line
	 * @param[in] sig - type signature
	 */
	explicit ProcessPipeFactory(const string_type & binPath, const string_type & cmdLine, const Type::RawTag & sig);
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	ProcessPipeFactory(const ProcessPipeFactory & o);
	
	/**
	 * Destructor.
	 */
	~ProcessPipeFactory() {
		if (this->args != NULL) {
			free(this->args);
			this->args = NULL;
		}
	}
	
	/**
	 * Executes the stored command-line and returns the new process handle.
	 *
	 * @param[in] mode - process open modes
	 * @return process handle
	 * @see tFdioPMode
	 */
	ProcessPipe run(tFdioPMode mode = (FDIO_USE_STDOUT | FDIO_BINARY_PIPE | FDIO_COMBINE)) const {
		if ( this->raw ) mode = mode | FDIO_RAW_CMDLINE;
#ifdef PCF_IS_WIN
		return ProcessPipe(fdious_popen(this->binaryPath.c_str(), const_cast<const char_type **>(this->args), NULL, NULL, mode));
#else /* ! PCF_IS_WIN */
		return ProcessPipe(fdios_popen(this->binaryPath.c_str(), const_cast<const char_type **>(this->args), NULL, NULL, mode));
#endif /* PCF_IS_WIN */
	}
	
	/**
	 * Executes the stored command-line with the passed argument and returns the new process handle.
	 *
	 * @param[in] arg - use the internal command-line as shell and pass this argument as single argument at the end
	 * @param[in] mode - process open modes
	 * @return process handle
	 * @see tFdioPMode
	 */
	ProcessPipe run(const string_type & arg, tFdioPMode mode = (FDIO_USE_STDOUT | FDIO_BINARY_PIPE | FDIO_COMBINE)) const {
		if ( this->raw ) mode = mode | FDIO_RAW_CMDLINE;
#ifdef PCF_IS_WIN
		return ProcessPipe(fdious_popen(this->binaryPath.c_str(), const_cast<const char_type **>(this->args), arg.c_str(), NULL, mode));
#else /* ! PCF_IS_WIN */
		return ProcessPipe(fdios_popen(this->binaryPath.c_str(), const_cast<const char_type **>(this->args), arg.c_str(), NULL, mode));
#endif /* PCF_IS_WIN */
	}
	
	/**
	 * Executes the stored command-line with the passed argument and returns the new process handle.
	 * The given file descriptor is used as input for the new process. This cannot be combined with FDIO_USE_STDIN.
	 *
	 * @param[in] arg - use the internal command-line as shell and pass this argument as single argument at the end
	 * @param[in,out] input - input file descriptor
	 * @param[in] mode - process open modes
	 * @return process handle
	 * @see tFdioPMode
	 */
	ProcessPipe run(const string_type & arg, FILE * input, tFdioPMode mode = (FDIO_USE_STDOUT | FDIO_BINARY_PIPE | FDIO_COMBINE)) const {
		if ( this->raw ) mode = mode | FDIO_RAW_CMDLINE;
#ifdef PCF_IS_WIN
		return ProcessPipe(fdious_popen(this->binaryPath.c_str(), const_cast<const char_type **>(this->args), arg.c_str(), input, mode));
#else /* ! PCF_IS_WIN */
		return ProcessPipe(fdios_popen(this->binaryPath.c_str(), const_cast<const char_type **>(this->args), arg.c_str(), input, mode));
#endif /* PCF_IS_WIN */
	}
	
	/**
	 * Splits the given string into an argument vector using shell syntax.
	 *
	 * @param[in] cmdLine - input command-line string to split
	 * @param[in] sig - type signature
	 * @param[out] outVec - output argument vector
	 */
	template <typename CharT>
	static void split(const std::basic_string<CharT> & cmdLine, const Type::ShellTag & sig, std::vector< std::basic_string<CharT> > & outVec);
private:
	/**
	 * Construct object from argument list vector.
	 *
	 * @param[in] procArgs - command-line arguments
	 */
	void initFromVector(const std::vector<string_type> & procArgs);
};


template <>
void ProcessPipeFactory::split(const std::basic_string<char> & cmdLine, const ProcessPipeFactory::Type::ShellTag & sig, std::vector< std::basic_string<char> > & outVec);

template <>
void ProcessPipeFactory::split(const std::basic_string<wchar_t> & cmdLine, const ProcessPipeFactory::Type::ShellTag & sig, std::vector< std::basic_string<wchar_t> > & outVec);


} /* namespace process */
} /* namespace pcf */


#endif /* __LIBPCFXX_PROCESS_UTILITY_HPP__ */
