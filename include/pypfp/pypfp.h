#ifndef __PYPFP_H__
#define __PYPFP_H__

#include <vector>
#include <string>

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>

#include <pfp/tokenizer.h>
#include <pfp/state_list.hpp>
#include <pfp/lexicon.hpp>
#include <pfp/unary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/pcfg_parser.hpp>

namespace com { namespace wavii { namespace pfp {

/**
 * pypfp wraps pfp with python glue
 */
class pypfp
{
private:

  tokenizer                    tokenizer_;
  state_list                   states_;
  lexicon                      lexicon_;
  unary_grammar                ug_;
  binary_grammar               bg_;
  pcfg_parser                  pcfg_;
  boost::shared_ptr<workspace> pworkspace_;

  void init(size_t sentence_length = 45, const std::string & data_dir = "");

public:

  pypfp();

  pypfp(size_t sentence_length);

  pypfp(size_t sentence_length, const std::string & data_dir);

  std::string parse(const std::string & sentence);
};

}}} // com::wavii::pfp

#endif // __PYPFP_H__
