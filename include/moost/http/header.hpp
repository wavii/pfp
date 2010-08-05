#ifndef __MOOST_HTTP_HEADER_HPP__
#define __MOOST_HTTP_HEADER_HPP__

#include <string>

namespace moost { namespace http {

struct header
{
  std::string name;
  std::string value;
};

}} // moost::http

#endif // __MOOST_HTTP_HEADER_HPP__
