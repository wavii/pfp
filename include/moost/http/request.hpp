#ifndef __MOOST_HTTP_REQUEST_HPP__
#define __MOOST_HTTP_REQUEST_HPP__

#include <string>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>

#include "moost/http/header.hpp"

namespace moost { namespace http {

/// A request received from a client.
struct request
{
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  std::vector<header> headers;
  std::string content; // the content (body) of the request

  std::vector<header>::iterator find_header(const std::string & header_name)
  {
    std::vector<header>::iterator result;
    for (result = headers.begin(); result != headers.end(); ++result)
    {
      if (boost::algorithm::iequals(result->name, header_name))
        break;
    }
    return result;
  }
};

}} // moost::http

#endif // __MOOST_HTTP_REQUEST_HPP__
