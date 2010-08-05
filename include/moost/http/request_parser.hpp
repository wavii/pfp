#ifndef __MOOST_HTTP_REQUEST_PARSER_HPP__
#define __MOOST_HTTP_REQUEST_PARSER_HPP__

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

#include "moost/http/request.hpp"

namespace moost { namespace http {

/// Parser for incoming requests.
class request_parser
{
public:
  /// Construct ready to parse the request method.
  request_parser();

  /// Reset to initial parser state.
  void reset();

  /// Parse some data. The tribool return value is true when a complete request
  /// has been parsed, false if the data is invalid, indeterminate when more
  /// data is required. The InputIterator return value indicates how much of the
  /// input has been consumed.
  template <typename InputIterator>
  boost::tuple<boost::tribool, InputIterator> parse(request& req,
      InputIterator begin, InputIterator end)
  {
    boost::tribool result = boost::indeterminate;
    if (state_ != header_end)
    {
      while (begin != end)
      {
        result = consume_header(req, *begin++);
        if (result)
          break;
        else if (!result)
          return boost::make_tuple(result, begin); // something wrong in the header
      }
    }
    if (state_ == header_end)
      result = consume_body(req, begin, end);
    return boost::make_tuple(result, begin);
  }

  template<typename InputIterator>
  boost::tribool consume_body(request & req, InputIterator begin, InputIterator end)
  {
    if (content_to_read_ < 0)
      return false; // probably bad content-length
    int content_read = std::min(static_cast<int>(end - begin), content_to_read_);
    req.content.append(begin, begin + content_read);
    begin += content_read;
    content_to_read_ -= content_read;
    if (content_to_read_ == 0)
      return true;
    else
      return boost::indeterminate;
  }

private:

  /// find and parse the content-length header
  boost::tribool parse_content_length(request & req);

  /// Handle the next character of input.
  boost::tribool consume_header(request& req, char input);

  /// Check if a byte is an HTTP character.
  static bool is_char(int c);

  /// Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  /// Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  /// Check if a byte is a digit.
  static bool is_digit(int c);

  /// The current state of the parser.
  enum state
  {
    method_start,
    method,
    uri_start,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3,
    header_end
  } state_;

  int content_to_read_;
};

}} // moost::http

#endif // __MOOST_HTTP_REQUEST_PARSER_HPP__
