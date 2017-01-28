/**
 * @file Stream.hpp
 * @author Daniel Starke
 * @copyright Copyright 2014-2017 Daniel Starke
 * @date 2014-10-11
 * @version 2016-12-23
 * @remarks This was not practical: http://sourceforge.net/p/mingw-w64/mailman/message/27708198/
 */
#ifndef __LIBPCFXX_FILE_STREAM_HPP__
#define __LIBPCFXX_FILE_STREAM_HPP__


#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4355)
#endif


namespace pcf {
namespace file {


namespace detail {


/* Forward declarations */
class WideInputFile;
class WideOutputFile;


/**
 * Wide character iostream device class.
 */
class WideFile : protected boost::iostreams::file_descriptor {
public:
	friend class WideInputFile;
	friend class WideOutputFile;
	typedef boost::iostreams::file_descriptor::handle_type handle_type;
	typedef wchar_t char_type;
	typedef boost::iostreams::file_descriptor::category category;

	/**
	 * Default constructor.
	 */
	WideFile() {
	}

	/**
	 * Constructor.
	 *
	 * @param[in] path - open file with this path
	 * @param[in] mode - file open mode
	 */
	explicit WideFile(const boost::iostreams::detail::path & path, BOOST_IOS::openmode mode = BOOST_IOS::in | BOOST_IOS::out) :
		boost::iostreams::file_descriptor(path, mode)
	{}

	/**
	 * Open specific file if no file is open yet for this object.
	 *
	 * @param[in] path - open file with this path
	 * @param[in] mode - file open mode
	 */
	template<typename Path>
	void open(const Path & path, BOOST_IOS::openmode mode = BOOST_IOS::in | BOOST_IOS::out) {
		this->boost::iostreams::file_descriptor::open(path, mode);
	}
	
	/**
	 * Returns whether a file is open or not. Use this to check
	 * if open() succeeded.
	 *
	 * @return open state
	 */
	bool is_open() const {
		return this->boost::iostreams::file_descriptor::is_open();
	}
	
	/**
	 * Close current open file.
	 */
	void close() {
		this->boost::iostreams::file_descriptor::close();
	}
	
	/**
	 * Read the given number of wide characters from the file.
	 *
	 * @param[out] s - output string pointer
	 * @param[in] n - maximum number of characters to read
	 * @return number of characters read from file or -1 on end of file
	 */
	std::streamsize read(char_type * s, std::streamsize n) {
		const boost::iostreams::stream_offset offset = static_cast<std::streamsize>(
			this->boost::iostreams::file_descriptor::read(
				reinterpret_cast<char *>(s),
				n * static_cast<std::streamsize>(sizeof(wchar_t))
			)
		);
		if (offset <= 0) return static_cast<std::streamsize>(offset);
		if ((offset % static_cast<std::streamsize>(sizeof(wchar_t))) != 0) {
			/* finished reading with incomplete wchar_t character -> rewind */
			this->boost::iostreams::file_descriptor::seek(
				-static_cast<boost::iostreams::stream_offset>(offset % static_cast<std::streamsize>(sizeof(wchar_t))),
				BOOST_IOS::cur
			);
		}
		return static_cast<std::streamsize>(
			static_cast<size_t>(offset - static_cast<boost::iostreams::stream_offset>(offset % static_cast<std::streamsize>(sizeof(wchar_t)))) / sizeof(wchar_t)
		);
	}
	
	/**
	 * Write the given number of wide characters to the file.
	 *
	 * @param[in] s - input string pointer
	 * @param[in] n - maximum number of characters to write
	 * @return number of characters written to the file
	 */
	std::streamsize write(const char_type * s, std::streamsize n) {
		const boost::iostreams::stream_offset offset = this->boost::iostreams::file_descriptor::write(
			reinterpret_cast<const char *>(s),
			n * static_cast<std::streamsize>(sizeof(wchar_t))
		);
		if (offset <= 0) return static_cast<std::streamsize>(offset);
		if ((offset % static_cast<std::streamsize>(sizeof(wchar_t))) != 0) {
			/* finished writing with incomplete wchar_t character -> rewind */
			this->boost::iostreams::file_descriptor::seek(
				-static_cast<boost::iostreams::stream_offset>(offset % static_cast<std::streamsize>(sizeof(wchar_t))),
				BOOST_IOS::cur
			);
		}
		return static_cast<std::streamsize>(
			static_cast<size_t>(offset - static_cast<boost::iostreams::stream_offset>(offset % static_cast<std::streamsize>(sizeof(wchar_t)))) / sizeof(wchar_t)
		);
	}
	
	/**
	 * Changes the current file pointer.
	 * Use std::ios_base::beg, std::ios_base::cur or std::ios_base::end to
	 * change the way off is interpreted.
	 * 
	 * @param[in] off - seeking offset
	 * @param[in] way - way of seeking by the given offset
	 * @return new file pointer
	 */
    std::streamoff seek(boost::iostreams::stream_offset off, BOOST_IOS::seekdir way) {
		return static_cast<std::streamoff>(
			this->boost::iostreams::file_descriptor::seek(
				off * static_cast<std::streamsize>(sizeof(wchar_t)), way
			) / static_cast<std::streamoff>(sizeof(wchar_t))
		);
	}
};


/**
 * Wide character istream device class.
 *
 * @see WideFile
 */
class WideInputFile : private WideFile {
public:
	typedef boost::iostreams::file_descriptor_source::handle_type handle_type;
	typedef wchar_t char_type;
	typedef boost::iostreams::file_descriptor_source::category category;

	/**
	 * Default constructor.
	 */
	WideInputFile() {
	}

	/**
	 * Constructor.
	 *
	 * @param[in] path - open file with this path
	 * @param[in] mode - file open mode
	 */
	explicit WideInputFile(const boost::iostreams::detail::path & path, BOOST_IOS::openmode mode = BOOST_IOS::in) :
		WideFile(path, mode)
	{}

	/**
	 * Open specific file if no file is open yet for this object.
	 *
	 * @param[in] path - open file with this path
	 * @param[in] mode - file open mode
	 */
	template<typename Path>
	void open(const Path & path, BOOST_IOS::openmode mode = BOOST_IOS::in) {
		this->WideFile::open(path, mode);
	}
	
	/* put these methods into public scope of this class */
    using WideFile::is_open;
    using WideFile::close;
    using WideFile::read;
    using WideFile::seek;
};


/**
 * Wide character ostream device class.
 *
 * @see WideFile
 */
class WideOutputFile : private WideFile {
public:
	typedef boost::iostreams::file_descriptor_sink::handle_type handle_type;
	typedef wchar_t char_type;
	typedef boost::iostreams::file_descriptor_sink::category category;

	/**
	 * Default constructor.
	 */
	WideOutputFile() {
	}

	/**
	 * Constructor.
	 *
	 * @param[in] path - open file with this path
	 * @param[in] mode - file open mode
	 */
	explicit WideOutputFile(const boost::iostreams::detail::path & path, BOOST_IOS::openmode mode = BOOST_IOS::out) :
		WideFile(path, mode)
	{}

	/**
	 * Open specific file if no file is open yet for this object.
	 *
	 * @param[in] path - open file with this path
	 * @param[in] mode - file open mode
	 */
	template<typename Path>
	void open(const Path & path, BOOST_IOS::openmode mode = BOOST_IOS::out) {
		this->WideFile::open(path, mode);
	}
	
	/* put these methods into public scope of this class */
    using WideFile::is_open;
    using WideFile::close;
    using WideFile::write;
    using WideFile::seek;
};


} /* namespace detail */


/**
 * Input file stream buffer.
 *
 * This class is designed to handle boost::filesystem::path correctly.
 * It also handles Unicode file paths correctly on Windows.
 *
 * @tparam Device - underlaying device type
 * @tparam CharT - device character type
 */
template <typename Device, typename CharT>
class basic_ifstream : public boost::iostreams::stream_buffer<Device>, public std::basic_istream<CharT> {
public:
    typedef CharT char_type;
    typedef typename std::char_traits<CharT> traits_type;

	/**
	 * Default constructor.
	 */
	basic_ifstream() :
		boost::iostreams::stream_buffer<Device>(),
		std::basic_istream<CharT>(this)
	{}

	/**
	 * Constructor.
	 * Opens the given file on construction.
	 *
	 * @param[in] p - path of file to open
	 * @param[in] mode - file open mode (std::ios_base::in is implicit)
	 */
	explicit basic_ifstream(const boost::filesystem::path & p, BOOST_IOS::openmode mode = BOOST_IOS::in) :
		boost::iostreams::stream_buffer<Device>(
			boost::iostreams::detail::path(p), mode | BOOST_IOS::in
		),
		std::basic_istream<CharT>(this)
	{}

	/**
	 * Opens the given file.
	 *
	 * @param[in] p - path of file to open
	 * @param[in] mode - file open mode (std::ios_base::in is implicit)
	 */
	void open(const boost::filesystem::path & p, BOOST_IOS::openmode mode = BOOST_IOS::in) {
		this->boost::iostreams::stream_buffer<Device>::open(
			boost::iostreams::detail::path(p), mode | BOOST_IOS::in
		);
		this->rdbuf(this);
	}
};


/**
 * Output file stream buffer.
 *
 * This class is designed to handle boost::filesystem::path correctly.
 * It also handles Unicode file paths correctly on Windows.
 *
 * @tparam Device - underlaying device type
 * @tparam CharT - device character type
 */
template <typename Device, typename CharT>
class basic_ofstream : public boost::iostreams::stream_buffer<Device>, public std::basic_ostream<CharT> {
public:
    typedef CharT char_type;
    typedef typename std::char_traits<CharT> traits_type;

	/**
	 * Default constructor.
	 */
	basic_ofstream() :
		boost::iostreams::stream_buffer<Device>(),
		std::basic_ostream<CharT>(this)
	{}

	/**
	 * Constructor.
	 * Opens the given file on construction.
	 *
	 * @param[in] p - path of file to open
	 * @param[in] mode - file open mode (std::ios_base::out is implicit)
	 */
	explicit basic_ofstream(const boost::filesystem::path & p, BOOST_IOS::openmode mode = BOOST_IOS::out) :
		boost::iostreams::stream_buffer<Device>(
			boost::iostreams::detail::path(p), mode | BOOST_IOS::out
		),
		std::basic_ostream<CharT>(this)
	{}

	/**
	 * Opens the given file.
	 *
	 * @param[in] p - path of file to open
	 * @param[in] mode - file open mode (std::ios_base::out is implicit)
	 */
	void open(const boost::filesystem::path & p, BOOST_IOS::openmode mode = BOOST_IOS::out) {
		this->boost::iostreams::stream_buffer<Device>::open(
			boost::iostreams::detail::path(p), mode | BOOST_IOS::out
		);
		this->rdbuf(this);
	}
};


/**
 * I/O file stream buffer.
 *
 * This class is designed to handle boost::filesystem::path correctly.
 * It also handles Unicode file paths correctly on Windows.
 *
 * @tparam Device - underlaying device type
 * @tparam CharT - device character type
 */
template <typename Device, typename CharT>
class basic_fstream : public boost::iostreams::stream_buffer<Device>, public std::basic_iostream<CharT> {
public:
    typedef CharT char_type;
    typedef typename std::char_traits<CharT> traits_type;

	/**
	 * Default constructor.
	 */
	basic_fstream() :
		boost::iostreams::stream_buffer<Device>(),
		std::basic_iostream<CharT>(this)
	{}

	/**
	 * Constructor.
	 * Opens the given file on construction.
	 *
	 * @param[in] p - path of file to open
	 * @param[in] mode - file open mode
	 */
	explicit basic_fstream(const boost::filesystem::path & p, BOOST_IOS::openmode mode = BOOST_IOS::in | BOOST_IOS::out) :
		boost::iostreams::stream_buffer<Device>(
			boost::iostreams::detail::path(p), mode
		),
		std::basic_iostream<CharT>(this)
	{}

	/**
	 * Opens the given file.
	 *
	 * @param[in] p - path of file to open
	 * @param[in] mode - file open mode
	 */
	void open(const boost::filesystem::path & p, BOOST_IOS::openmode mode = BOOST_IOS::in | BOOST_IOS::out) {
		this->boost::iostreams::stream_buffer<Device>::open(
			boost::iostreams::detail::path(p), mode
		);
		this->rdbuf(this);
	}
};


/** Possible replacement type for std::ifstream when used with boost::filesystem::path. */
typedef basic_ifstream<boost::iostreams::file_descriptor_source, char> ifstream;
/** Possible replacement type for std::ofstream when used with boost::filesystem::path. */
typedef basic_ofstream<boost::iostreams::file_descriptor_sink, char> ofstream;
/** Possible replacement type for std::fstream when used with boost::filesystem::path. */
typedef basic_fstream<boost::iostreams::file_descriptor, char> fstream;


/** Possible replacement type for std::wifstream when used with boost::filesystem::path. */
typedef basic_fstream<detail::WideInputFile, wchar_t> wifstream;
/** Possible replacement type for std::wofstream when used with boost::filesystem::path. */
typedef basic_fstream<detail::WideOutputFile, wchar_t> wofstream;
/** Possible replacement type for std::wfstream when used with boost::filesystem::path. */
typedef basic_fstream<detail::WideFile, wchar_t> wfstream;


/* alternative implementation; disadvantage: not seekable
typedef basic_ifstream<boost::iostreams::code_converter<
	boost::iostreams::file_descriptor_source,
	pcf::coding::WideConvert
>, wchar_t> wifstream;
typedef basic_ofstream<boost::iostreams::code_converter<
	boost::iostreams::file_descriptor_sink,
	pcf::coding::WideConvert
>, wchar_t> wofstream;
typedef basic_fstream<boost::iostreams::code_converter<
	boost::iostreams::file_descriptor,
	pcf::coding::WideConvert
>, wchar_t> wfstream;
*/


} /* namespace file */
} /* namespace pcf */


#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif /* __LIBPCFXX_FILE_STREAM_HPP__ */
