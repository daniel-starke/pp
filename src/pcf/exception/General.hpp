/**
 * @file General.hpp
 * @author Daniel Starke
 * @copyright Copyright 2013-2018 Daniel Starke
 * @date 2013-07-04
 * @version 2017-10-31
 */
#ifndef __LIBPCFXX_EXCEPTION_GENERAL_HPP__
#define __LIBPCFXX_EXCEPTION_GENERAL_HPP__

#include <string>
#include <stdexcept>
#include <boost/exception/all.hpp>


namespace pcf {
namespace exception {


struct Api : virtual std::exception, virtual boost::exception {};

struct SystemError : virtual Api {};

struct Range : virtual Api {};
struct OutOfRange : virtual Range {};
struct UnsupportedRange : virtual Range {};

struct Type : virtual Api {};
struct UnsupportedType : virtual Type {};

struct Data : virtual Api {};
struct DataNotFound : virtual Data {};
struct DataFormat : virtual Data {};
struct DataIncomplete : virtual Data {};
struct InvalidValue : virtual Data {};
struct NullPointer : virtual Data {};
struct OutOfMemory : virtual Data {};

struct Math : virtual Api {};
struct DivisionByZero : virtual Math {};

struct File : virtual Api {};
struct FileNotFound : virtual File {};
struct AccessDenied : virtual File {};

struct Script : virtual Api {};
struct SyntaxError : virtual Script {};
struct SymbolUnknown : virtual Script {};

struct InputOutput : virtual Api {};
struct Input : virtual InputOutput {};
struct Output : virtual InputOutput {};

struct Database : virtual Api {};
struct SQLite3 : virtual Database {};

struct State : virtual Api {};
struct InvalidState : virtual State {};

struct Initialization : virtual Api {};
struct UnInitialized : virtual Initialization {};


namespace tag {


typedef boost::error_info<struct TagMessage, std::string> Message;
typedef boost::error_info<struct TagErrorCode, int> ErrorCode;


} /* namespace tag */


} /* namespace exception */
} /* namespace pcf */


#endif /* __LIBPCFXX_EXCEPTION_GENERAL_HPP__ */
