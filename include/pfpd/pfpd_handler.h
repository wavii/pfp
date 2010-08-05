#ifndef __PFPD_HANDLER_H__
#define __PFPD_HANDLER_H__

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include <moost/http.hpp>
#include "resource_stack.hpp"

#include <pfp/tokenizer.h>
#include <pfp/state_list.hpp>
#include <pfp/lexicon.hpp>
#include <pfp/unary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/pcfg_parser.hpp>

namespace com { namespace wavii { namespace pfp {

class pfpd_handler : public moost::http::request_handler_base<pfpd_handler>
{
private:

  tokenizer tokenizer_;
  state_list states_;
  lexicon lexicon_;
  unary_grammar ug_;
  binary_grammar bg_;
  pcfg_parser pcfg_;
  size_t timer_bucket_size_;
  resource_stack< workspace > workspaces_;

  // perform utf-8 encoded URL-decoding on a string. returns false if the encoding was invalid
  static bool url_decode(const std::string& in, std::string& out);

  // provide a version string
  std::string version();

  // provide a little get-console for interactive parsing
  std::string console(const std::string & query);

  // tokenize, lexicon-weight, and parse a sentence
  std::string parse(const std::string & sentence);

public:

  pfpd_handler();

  void init(size_t sentence_length, size_t threads, const std::string & data_dir);

  void handle_request(const moost::http::request& req, moost::http::reply& rep);

};

}}} // com::wavi::pfp

#endif // __PFPD_HANDLER_H__
