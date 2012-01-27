#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

#include <pfp/config.h>
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

int main(int argc, char * argv[])
{
  std::clog << "pfpc_token: command line interface for pfp!" << std::endl;
  std::clog << "build: " << __DATE__ << " (" << __TIME__ << ") of pfp version " << consts::version << " (c) Wavii,Inc. 2012" << std::endl;
  std::clog << "usage: " << argv[0] << " <max sentence length=45> <data dir=/usr/share/pfp/>" << std::endl;

  size_t sentence_length = argc < 2 ? 45 : lexical_cast<size_t>(argv[1]);
  std::string data_dir = argc < 3 ? "/usr/share/pfp/" : argv[2]; // make install copies files to /usr/share/pfp by default

  state_list states;
  lexicon lexicon(states);
  unary_grammar ug(states);
  binary_grammar bg(states);
  pcfg_parser pcfg(states, ug, bg);

  std::clog << "loading lexicon and grammar" << std::endl;
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
  workspace w(sentence_length, states.size());

  std::vector< std::string > words;
  std::clog << "ready!  enter each token per line, empty line to finish the sentence:" << std::endl;
  for (std::string word; std::getline(std::cin, word); ) {
    boost::trim(word);
    if (word.empty())
      break;
    words.push_back(word);
  }

  std::vector< std::pair< state_t, float > > state_weight;
  std::vector< std::vector< state_score_t > > sentence_f;
  node result;

  for (std::vector< std::string >::const_iterator it = words.begin(); it != words.end(); ++it)
  {
    state_weight.clear(); 
    lexicon.score(*it, std::back_inserter(state_weight));
    sentence_f.push_back(std::vector< state_score_t >(state_weight.size()));
    // scale by score_resolution in case we are downcasting our weights
    for (size_t i = 0; i != state_weight.size(); ++i)
      sentence_f.back()[i] = state_score_t(state_weight[i].first, state_weight[i].second * consts::score_resolution);
  }
  // add the boundary symbol
  sentence_f.push_back( std::vector< state_score_t >(1, state_score_t(consts::boundary_state, 0.0f)));
  // and parse!
  if (!pcfg.parse(sentence_f, w, result))
    std::cout << "Sorry, pfp couldn't work out the result!" << std::endl;
  // stitch together the results
  std::ostringstream oss;
  std::vector< std::string >::iterator word_it = words.begin();
  stitch(oss, result, word_it, states);
  std::cout << oss.str() << std::endl;
}
