#ifndef __MOOST_HTTP_MIME_TYPES_HPP__
#define __MOOST_HTTP_MIME_TYPES_HPP__

#include <string>

namespace moost { namespace http { namespace mime_types {

/// Convert a file extension into a MIME type.
std::string extension_to_type(const std::string& extension);

}}} // moost::http::mime_types

#endif // __MOOST_HTTP_MIME_TYPES_HPP__
