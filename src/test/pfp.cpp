
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

#include <pfp/config.h>
#include <pfp/tokenizer.h>
#include <pfp/state_list.hpp>
#include <pfp/lexicon.hpp>
#include <pfp/unary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/pcfg_parser.hpp>

using namespace com::wavii::pfp;
using namespace boost;
namespace fs = boost::filesystem;

template<class T>
void load(T & obj, boost::filesystem::path p)
{
  if (!fs::exists(p))
    throw std::runtime_error("can't find " + p.string());
  std::ifstream in(p.string().c_str());
  obj.load(in);
}

BOOST_AUTO_TEST_SUITE( pfp_parser_test )

BOOST_AUTO_TEST_CASE( test_pfp_hash )
{
  tokenizer tokenizer;
  state_list states;
  lexicon lexicon(states);
  unary_grammar ug(states);
  binary_grammar bg(states);
  pcfg_parser pcfg(states, ug, bg);

  const std::string data_dir = "./share/pfp";
  load(tokenizer, fs::path(data_dir) / "americanizations");
  load(states, fs::path(data_dir) / "states");
  {
    fs::path ps[] = { fs::path(data_dir) / "words", fs::path(data_dir) / "sigs", fs::path(data_dir) / "word_state", fs::path(data_dir) / "sig_state" };
    std::ifstream ins[4];
    for (int i = 0; i != 4; ++i)
    {
      if (!fs::exists(ps[i]))
        throw std::runtime_error("can't find " + ps[i].string());
      ins[i].open(ps[i].string().c_str());
    }
    lexicon.load(ins[0], ins[1], ins[2], ins[3]);
  }
  load(ug, fs::path(data_dir) / "unary_rules");
  load(bg, fs::path(data_dir) / "binary_rules");
  workspace w(45, states.size());

  const std::string sentence= "Description This 2005 Nissan Altima available from Rama Auto Inc with Stock # 330051 is priced at $ 9500.00 .";
  const std::string parsed = "(ROOT (S (S (VP (VBG Description) (NP (DT This) (CD 2005) (NNP Nissan) (NNP Altima)) (ADJP (JJ available) (PP (IN from) (NP (NP (NNP Rama) (NNP Auto) (NNP Inc)) (PP (IN with) (NP (NNP Stock)))))))) (NP (# #) (CD 330051)) (VP (VBZ is) (VP-VBN-v (VBN priced) (PP (IN at) (NP ($ $) (CD 9500.00))))) (. .)) )";

   {
    std::vector< std::string > words;
    std::vector< std::pair< state_t, float > > state_weight;
    std::vector< std::vector< state_score_t > > sentence_f;
    node result;
    tokenizer.tokenize(sentence, words);
    for (std::vector< std::string >::const_iterator it = words.begin(); it != words.end(); ++it)
    {
      state_weight.clear(); lexicon.score(*it, std::back_inserter(state_weight));
      sentence_f.push_back(std::vector< state_score_t >(state_weight.size()));
      // scale by score_resolution in case we are downcasting our weights
      for (size_t i = 0; i != state_weight.size(); ++i)
        sentence_f.back()[i] = state_score_t(state_weight[i].first, state_weight[i].second * consts::score_resolution);
    }
    // add the boundary symbol
    sentence_f.push_back( std::vector< state_score_t >(1, state_score_t(consts::boundary_state, 0.0f)));
    // and parse!
    if (!pcfg.parse(sentence_f, w, result))
      BOOST_FAIL("Parsing shouldn't fail!");
    // stitch together the results
    std::ostringstream oss;
    std::vector< std::string >::iterator word_it = words.begin();
    stitch(oss, result, word_it, states);

    BOOST_CHECK_EQUAL(oss.str(), parsed);
   }
}

BOOST_AUTO_TEST_SUITE_END()
