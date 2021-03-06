/**
 * @file Target.hpp
 * @author Daniel Starke
 * @copyright Copyright 2013-2018 Daniel Starke
 * @date 2013-11-09
 * @version 2017-01-14
 */
#ifndef __LIBPCFXX_OS_TARGET_HPP__
#define __LIBPCFXX_OS_TARGET_HPP__


#include <string>
#include <boost/cstdint.hpp>


namespace pcf {
namespace os {


#include <libpcf/target.h>


#undef _U
#undef _US
#if defined(PCF_IS_WIN) && defined(_UNICODE)
#define _U(x) w ## x
#define _US(x) L ## x
#else /* ASCII */
#define _U(x) x
#define _US(x) x
#endif /* UNICODE */


typedef std::_U(string) tstring;


} /* namespace os */
} /* namespace pcf */


#endif /* __LIBPCFXX_OS_TARGET_HPP__ */
