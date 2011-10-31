#include <pypfp/pypfp.h>

#include <sstream>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <pfp/util.hpp>

using namespace com::wavii::pfp;
using namespace boost::python;
namespace fs = boost::filesystem;

template<class T>
void load(T & obj, fs::path p)
{
  if (!fs::exists(p))
    throw std::runtime_error("can't find " + p.string());
  std::ifstream in(p.string().c_str());
  obj.load(in);
}

pypfp::pypfp()
: lexicon_(states_),
ug_(states_),
bg_(states_),
pcfg_(states_, ug_, bg_)
{
  init();
}

pypfp::pypfp(size_t sentence_length)
: lexicon_(states_),
ug_(states_),
bg_(states_),
pcfg_(states_, ug_, bg_)
{
  init(sentence_length);
}

pypfp::pypfp(size_t sentence_length, const std::string & data_dir)
: lexicon_(states_),
ug_(states_),
bg_(states_),
pcfg_(states_, ug_, bg_)
{
  init(sentence_length, data_dir);
}

void pypfp::init(size_t sentence_length /*= 45*/, const std::string & data_dir /*= ""*/)
{
  std::string pfp_path = extract<std::string>(import("pfp").attr("__file__"));
  fs::path data_dir_p = data_dir.empty() ? fs::path(pfp_path).parent_path() / "share" : data_dir;

  load(tokenizer_, data_dir_p / "americanizations");
  load(states_, data_dir_p / "states");
  {
    fs::path ps[] = { data_dir_p / "words", data_dir_p / "sigs", data_dir_p / "word_state", data_dir_p / "sig_state" };
    std::ifstream ins[4];
    for (int i = 0; i != 4; ++i)
    {
      if (!fs::exists(ps[i]))
        throw std::runtime_error("can't find " + ps[i].string());
      ins[i].open(ps[i].string().c_str());
    }
    lexicon_.load(ins[0], ins[1], ins[2], ins[3]);
  }
  load(ug_, data_dir_p / "unary_rules");
  load(bg_, data_dir_p / "binary_rules");
  pworkspace_.reset(new workspace(sentence_length, states_.size()));
}

std::string pypfp::parse(const std::string & sentence)
{
  // now some words
  std::vector< std::string > words;
  std::vector< std::pair< state_t, float > > state_weight;
  std::vector< std::vector< state_score_t > > sentence_f;
  node result;
  tokenizer_.tokenize(sentence, words);
  for (std::vector< std::string >::const_iterator it = words.begin(); it != words.end(); ++it)
  {
    state_weight.clear(); lexicon_.score(*it, std::back_inserter(state_weight));
    sentence_f.push_back(std::vector< state_score_t >(state_weight.size()));
    // scale by score_resolution in case we are downcasting our weights
    for (size_t i = 0; i != state_weight.size(); ++i)
      sentence_f.back()[i] = state_score_t(state_weight[i].first, state_weight[i].second * consts::score_resolution);
  }
  // add the boundary symbol
  sentence_f.push_back( std::vector< state_score_t >(1, state_score_t(consts::boundary_state, 0.0f)));
  // and parse!
  if (!pcfg_.parse(sentence_f, *pworkspace_, result))
    return "";
  // stitch together the results
  std::ostringstream oss;
  stitch(oss, result, words.begin(), states_);
  return oss.str();
}

BOOST_PYTHON_MODULE(pfp)
{
    class_<pypfp, boost::noncopyable>("Parser", init<>())
          .def(init<size_t>())
          .def(init<size_t, const std::string &>())
      .def("parse", &pypfp::parse)
    ;
}
