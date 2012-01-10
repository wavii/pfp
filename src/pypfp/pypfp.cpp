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

std::string pypfp::_parse_tokens(const std::vector<std::string>& words)
{
  std::vector< std::pair< state_t, float > > state_weight;
  std::vector< std::vector< state_score_t > > sentence_f;
  node result;

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

std::string pypfp::parse_tokens(const boost::python::list& words)
{
  std::vector<std::string> words_vec;
  size_t len = boost::python::len(words);
  
  // makes a copy of the content. Yes, I could avoid it by iterating but then
  // I'd have to use stl_iterator and make parse_tokens templated.
  // It's not really worth the hassle.
  for (size_t i = 0; i != len; ++i)
    words_vec.push_back(boost::python::extract<std::string>(words[i]));

   return _parse_tokens(words_vec);
}

std::string pypfp::parse(const std::string & sentence)
{
  // now some words
  std::vector< std::string > words;
  tokenizer_.tokenize(sentence, words);
  return _parse_tokens(words);
}

BOOST_PYTHON_MODULE(pfp)
{
    class_<pypfp, boost::noncopyable>("Parser", init<>())
          .def(init<size_t>(boost::python::args("max_sentence_len")))
          .def(init<size_t, const std::string &>(boost::python::args("max_sentence_len", "data_dir")))
      .def("parse", &pypfp::parse, boost::python::args("self", "sentence"),
            "Will parse the given sentence")
      .def("parse_tokens", &pypfp::parse_tokens, boost::python::args("self", "tokens"),
            "Will parse the give tokens list")
    ;
}
