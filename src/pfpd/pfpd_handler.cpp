#include <pfpd/pfpd_handler.h>

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <pfp/config.h>
#include <pfp/util.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>

using namespace com::wavii::pfp;
using namespace moost::http;
namespace fs = boost::filesystem;

pfpd_handler::pfpd_handler()
: lexicon_(states_),
  ug_(states_),
  bg_(states_),
  pcfg_(states_, ug_, bg_)
{
}

// load a file and err reasonably if not found
template<class T>
void load(T & obj, boost::filesystem::path p)
{
  if (!boost::filesystem::exists(p))
    throw std::runtime_error("can't find " + p.string());
  std::clog << "loading " << p.string() << std::endl;
  std::ifstream in(p.string().c_str());
  obj.load(in);
}

void pfpd_handler::init(size_t sentence_length, size_t threads, const std::string & data_dir)
{
  timer_bucket_size_ = (sentence_length + 9) / 10;
  std::clog << "loading lexicon and grammar" << std::endl;
  load(tokenizer_, fs::path(data_dir) / "americanizations");
  load(states_, fs::path(data_dir) / "states");
  {
    fs::path ps[] = { fs::path(data_dir) / "words", fs::path(data_dir) / "sigs", fs::path(data_dir) / "word_state", fs::path(data_dir) / "sig_state" };
    std::ifstream ins[4];
    for (int i = 0; i != 4; ++i)
    {
      if (!fs::exists(ps[i]))
        throw std::runtime_error("can't find " + ps[i].string());
      ins[i].open(ps[i].string().c_str());
    }
    lexicon_.load(ins[0], ins[1], ins[2], ins[3]);
  }
  load(ug_, fs::path(data_dir) / "unary_rules");
  load(bg_, fs::path(data_dir) / "binary_rules");
  std::clog << "allocating " << threads << " workspaces of sentence-length " << sentence_length << std::endl;
  while (threads-- != 0)
    workspaces_.add_resource(new workspace(sentence_length, states_.size()));
}

bool pfpd_handler::url_decode(const std::string& in, std::string& out)
{
  // TODO: URL-encoding maps to ISO-9somethingsomething codepage
  // do i need to convert to utf-8 here or is everything kosher?
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value;
        if (std::istringstream(in.substr(i + 1, 2)) >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
          return false;
      }
      else
        return false;
    }
    else if (in[i] == '+')
      out += ' ';
    else
      out += in[i];
  }
  return true;
}

void pfpd_handler::handle_request(const moost::http::request& req, moost::http::reply& rep)
{
  std::string request_path;
  if (!url_decode(req.uri, request_path))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = "text/plain";

  try
  {
    rep.status = reply::ok;
    if (request_path == "/version" || request_path == "/version/")
      rep.content = version();
    else if (request_path.find("/console") == 0)
    {
      rep.content = console( request_path.substr(sizeof("/console") - 1) );
      rep.headers[1].value = "text/html";
    }
    else if (request_path.find("/parse/") == 0)
      rep.content = parse(request_path.substr(sizeof("/parse/") - 1));
    else
      rep = reply::stock_reply(reply::not_found);
  } catch (const std::runtime_error & e)
  {
    std::cerr << "error: " << e.what() << std::endl;
    rep.content = "";
  }

  // and finally the length
  rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
}

std::string pfpd_handler::version()
{
  const char * e[] =
  {
    "sometimes when i freestyle, i lose confidence.",
    "i do not have any money so am sending you this drawing i did of a spider instead.",
    "deleted code is debugged code.",
    "the computing scientist's main challenge is not to get confused by the complexities of his own making.",
    "debugging is twice as hard as writing the code in the first place. therefore, if you write the code as cleverly as possible, you are, by definition, not smart enough to debug it.",
    "censorship is telling a man he can't have a steak just because a baby can't chew it.",
    "whenever you find yourself on the side of the majority, it is time to pause and reflect.",
    "if you don't know where you're going, any road will get you there.",
    "if you ever drop your keys into a river of molten lava, let'em go, because, man, they're gone.",
    "it takes a big man to cry, but it takes a bigger man to laugh at that man.",
    "consider the daffodil. and while you're doing that, i'll be over here, looking through your stuff."
  };
  std::ostringstream oss;
  oss << "pfp version " << consts::version << ", build: "  << __DATE__ << " (" << __TIME__ << ")";
  oss << "\n\n" << e[rand() % (sizeof(e) / sizeof(const char *))];
  return oss.str();
}

std::string pfpd_handler::console(const std::string & query)
{
  std::ostringstream oss;
  std::string q;
  if (query.find("?q=") != std::string::npos)
    q = boost::algorithm::trim_copy(query.substr(query.find("?q=") + 3));
  oss << "<html>\n<title>pfp console</title>\n<body><h1>Hey everybody let's parse!</h1>\n<form action=\"/console/\" method=\"get\">";
  oss << "<textarea rows=\"10\" cols=\"80\" name=\"q\">";
  oss << q << std::endl;
  oss << "</textarea><br/><input type=\"submit\" value=\"parse\"/>\n</form>\n";
  if (!q.empty())
  {
    std::string o;
    try { o = parse(q); }
    catch (const std::runtime_error & e) { o = std::string("whoops: ") + e.what(); }
    oss << "<p/><h3>result:</h3>" << o << std::endl;
  }
  oss << "\n</body></html>";
  return oss.str();
}

std::string pfpd_handler::parse(const std::string & sentence)
{
  // befirst, get a workspace
  resource_stack<workspace>::scoped_resource pw(workspaces_);
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
  if (!pcfg_.parse(sentence_f, *pw, result))
    return "";
  // stitch together the results
  std::ostringstream oss;
  stitch(oss, result, words.begin(), states_);
  return oss.str();
}

