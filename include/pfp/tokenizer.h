#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <vector>
#include <string>
#include <fstream>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace com { namespace wavii { namespace pfp {

// lexer/tokenizer that fits fairly close to Penn Treebank conventions.
// this wraps the lexer rules in etc/tokenizer.flex
class tokenizer
{
private:

  struct replacement
  {
    boost::regex find;
    std::string format;
    boost::optional< boost::regex > unless;
    replacement(std::string find_, std::string format_) : find(find_), format(format_) {}
    replacement(std::string find_, std::string format_, std::string unless_) : find(find_), format(format_), unless(boost::regex(unless_)) {}
  };

  boost::unordered_map<std::string, std::string> m_literals;
  std::vector< replacement >                     m_replacements;

  // convert colour to color
  // british, canadian - doesn't matter.  deep down we're all american.
  std::string americanize(const std::string & in) const
  {
    boost::unordered_map<std::string, std::string>::const_iterator it_h = m_literals.find(in);
    if (it_h != m_literals.end())
      return it_h->second;
    boost::smatch mr;
    for (std::vector<replacement>::const_iterator it = m_replacements.begin(); it != m_replacements.end(); ++it)
    {
      if (   !(it->unless && boost::regex_match(in, *it->unless))
          && boost::regex_match(in, mr, it->find))
        return mr.format(it->format);
    }
    return in;
  }

  // convert certain windows codepage peculiarities that we shouldn't
  // have to see, but may crop up anyway
  std::string cp1252_normalize(const std::string & in) const
  {
    std::string ret = in;
    boost::algorithm::replace_all(ret, "&apos;", "'");
    boost::algorithm::replace_all(ret, "\xc2\x91", "`");
    boost::algorithm::replace_all(ret, "\xe2\x80\x98", "`");
    boost::algorithm::replace_all(ret, "\xc2\x92", "'");
    boost::algorithm::replace_all(ret, "\xe2\x80\x99", "'");
    boost::algorithm::replace_all(ret, "\xc2\x93", "``");
    boost::algorithm::replace_all(ret, "\xe2\x80\x9c", "``");
    boost::algorithm::replace_all(ret, "\xc2\x94", "''");
    boost::algorithm::replace_all(ret, "\xe2\x80\x9d", "''");
    boost::algorithm::replace_all(ret, "\xc2\xbc", "1\\/4");
    boost::algorithm::replace_all(ret, "\xc2\xbd", "1\\/2");
    boost::algorithm::replace_all(ret, "\xc2\xbe", "3\\/4");
    boost::algorithm::replace_all(ret, "\xc2\xa2", "cents");
    boost::algorithm::replace_all(ret, "\xc2\xa3", "#");
    boost::algorithm::replace_all(ret, "\xc2\x80", "$");
    boost::algorithm::replace_all(ret, "\xe2\x82\xac", "$");  // Euro -- no good translation!
    return ret;
  }

  // escape x -> \x
  std::string escape(const std::string & in, char echar) const
  {
    std::string ret = in;
    std::string c(1, echar), ec = "\\" + c, eec = "\\\\" + c;
    // change c -> \c
    boost::algorithm::replace_all(ret, c, ec);
    // change \\c -> \c
    boost::algorithm::replace_all(ret, eec, ec);
    return ret;
  }

  std::string ampersandize(const std::string & in) const
  {
    return boost::algorithm::ireplace_all_copy(in, "&amp;", "&");
  }

  void init()
  {
    m_replacements.push_back(replacement("(.*)haem(at)?o(.*)", "$1hem$2o$3"));
    m_replacements.push_back(replacement("(.*)aemia$", "$1emia"));
    m_replacements.push_back(replacement("(.*)([lL]euk)aem(.*)", "$1$2em$3"));
    m_replacements.push_back(replacement("(.*)programme(s?)$", "$1program$2"));
    m_replacements.push_back(replacement("^([a-z]{3,})our(s?)$", "$1or$2", "glamour|de[tv]our"));
  }

public:

  class tokenizer_out
  {
  private:
    std::vector<std::string> & m_out;
    const com::wavii::pfp::tokenizer & m_t;
  public:
    tokenizer_out(std::vector<std::string> & out, const com::wavii::pfp::tokenizer & t) : m_out(out), m_t(t) {}
    void put(const char * p) { m_out.push_back(p); }
    void put_american(const char * p) { m_out.push_back(m_t.americanize(p)); }
    void put_cp1252(const char * p) { m_out.push_back(m_t.cp1252_normalize(p)); }
    void put_escape(const char * p, char echar) { m_out.push_back(m_t.escape(p, echar)); }
    void put_amp(const char * p) { m_out.push_back(m_t.ampersandize(p)); }
    void err(const char * p) { /* do zilch */ }
  };

  void tokenize(const std::string & in, std::vector<std::string> & out) const;

  tokenizer()
  {
    init();
  }

  tokenizer(const std::string & americanize_path)
  {
    init();
    std::ifstream in(americanize_path.c_str());
    load(in);
  }

  void load(std::istream & in)
  {
    std::pair< std::string, std::string > kv;
    m_literals.clear();
    while (in >> kv.first >> kv.second)
      m_literals.insert(kv);
   }
};


}}} // com::wavii::pfp

#endif // __TOKENIZER_H__
