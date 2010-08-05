#ifndef __LEXICON_HPP__
#define __LEXICON_HPP__

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>

// to get libicu goodness
#include <unicode/ustring.h>
// for ostream support
#include <unicode/ustream.h>
#include <unicode/uchar.h>

#include <pfp/config.h>
#include <pfp/state_list.hpp>
#include <boost/unordered_map.hpp>

namespace com { namespace wavii { namespace pfp {

// provide a conditional probability P(state | word)
// we try to calculate the conditional probability by looking
// the word up in our lexicon.  if we can't find the word, we
// try looking up a simple, high-level signature instead
class lexicon
{
private:

  const state_list &                                                                m_states;
  std::vector< std::vector< std::pair< state_t, count_t > > >                       m_word_state;
  std::vector< std::vector< std::pair< state_t, count_t > > >                       m_sig_state;
  boost::unordered_map< std::string, word_t >                                       m_word_index;
  boost::unordered_map< std::string, word_t >                                       m_sig_index;
  std::vector< count_t >                                                            m_word;
  std::vector< count_t >                                                            m_sig;
  std::vector< count_t >                                                            m_known_state;
  std::vector< count_t >                                                            m_unknown_state;
  count_t                                                                           m_known;
  count_t                                                                           m_unknown;
  std::vector< std::pair< state_t, count_t > >                                      m_open_class;
  std::vector< std::pair< state_t, count_t > >                                      m_any;
  std::vector< std::pair< state_t, count_t > >                                      m_none;

  struct cmp_state_t_count_t
  {
    bool operator()(const std::pair< state_t, count_t > & lhs, const std::pair< state_t, count_t > & rhs) const
    { return lhs.first < rhs.first; }
  };

  // outer join of major + minor, with preference for major at intersecting states
  void merge_states(const std::vector< std::pair< state_t, count_t > > & major,
                    const std::vector< std::pair< state_t, count_t > > & minor,
                    std::vector< std::pair< state_t, count_t > > & out)
  {
    std::vector< std::pair< state_t, count_t > >::const_iterator it_maj = major.begin(), end_maj = major.end(),
                                                                 it_min = minor.begin(), end_min = minor.end();
    while (it_maj != end_maj && it_min != end_min)
    {
      if (it_maj->first < it_min->first)
        out.push_back(*it_maj++);
      else if (it_min->first < it_maj->first)
        out.push_back(*it_min++);
      else
        out.push_back(*it_maj++), ++it_min;
    }
    while (it_maj != end_maj)
      out.push_back(*it_maj++);
    while (it_min != end_min)
      out.push_back(*it_min++);
  }

  // right join major + minor, with preference for major at intersecting states
  void right_intersect_states(const std::vector< std::pair< state_t, count_t > > & major,
                        const std::vector< std::pair< state_t, count_t > > & minor,
                        std::vector< std::pair< state_t, count_t > > & out)
  {
    std::vector< std::pair< state_t, count_t > >::const_iterator it_maj = major.begin(), end_maj = major.end(),
                                                                 it_min = minor.begin(), end_min = minor.end();
    while (it_maj != end_maj && it_min != end_min)
    {
      if (it_maj->first < it_min->first)
        ++it_maj;
      else if (it_min->first < it_maj->first)
        out.push_back(*it_min++);
      else
        out.push_back(*it_maj++), ++it_min;
    }
    while (it_min != end_min)
      out.push_back(*it_min++);
  }

  // get a very coarse signature of a word, consisting of whether
  // - the word is all caps
  // - the word is capitalized as the first word of a sentence
  // - the word is capitalized as not the first word
  // - the word is lowercase
  // - the word has a dash
  // - the word contains digits
  // - the word is a number
  // - the final letter of the word
  std::string get_sig(const std::string & word, int pos)
  {
    UnicodeString us(word.c_str());
    bool digit = false;
    bool nondigit = false;
    bool lower = false;

    std::ostringstream oss;
    oss << "UNK";
    int len = us.length();

    for (int i = 0; i != len; ++i)
    {
      if (u_isdigit(us[i]))
        digit = true;
      else
      {
        nondigit = true;
        if (u_isalpha(us[i]) && (u_islower(us[i]) || u_istitle(us[i])))
          lower = true;
      }
    }
    if (len > 0 && (u_isupper(us[0]) || u_istitle(us[0])))
    {
      if (!lower)
        oss << "-ALLC";
      else if (pos == 0)
        oss << "-INIT";
      else
        oss << "-UC";
    }
    else if (lower)
      oss << "-LC";
    if (us.indexOf('-') > -1)
      oss << "-DASH";
    if (digit)
    {
      if (nondigit)
        oss << "-DIG";
      else
        oss << "-NUM";
    }
    else if (len > 3)
      oss << UnicodeString(u_tolower(us[len - 1]));

    return oss.str();
  }

  // get the conditional probability for a word by looking it up in our lexicon
  // if smooth is true, add in some other potential states with an unknown probability
  template <class OutputIterator>
  void word_score(word_t word, OutputIterator out, bool smooth)
  {
    std::vector< std::pair< state_t, count_t > > states;
    merge_states(m_word_state[word], smooth ? m_any : m_none, states);

    for (std::vector< std::pair< state_t, count_t > >::const_iterator it = states.begin(); it != states.end(); ++it)
    {
      float cw = m_word[word];
      float ct = m_known_state[it->first];
      float pbtw = smooth ? (it->second + consts::word_smooth_factor * (m_unknown_state[it->first] / m_unknown) ) / (cw + consts::word_smooth_factor) : (it->second / cw);
      float pbwt = std::log(pbtw * cw / ct);
      if (pbwt > -100.0f)
        *out++ = std::make_pair(it->first, pbwt);
    }
  }

  // get the conditional probability for a word by building a signature out of it
  template <class OutputIterator>
  void sig_score(const std::string & word, OutputIterator out, int pos)
  {
    std::vector< std::pair< state_t, count_t > > states;
    boost::unordered_map< std::string, word_t >::iterator it_s = m_sig_index.find(get_sig(word, pos));
    if (it_s != m_sig_index.end())
      right_intersect_states(m_sig_state[it_s->second], m_open_class, states);
    else
      states = m_open_class;
    for (std::vector< std::pair< state_t, count_t > >::const_iterator it = states.begin(); it != states.end(); ++it)
    {
      float cs = it_s == m_sig_index.end() ? 0.0 : m_sig[it_s->second];
      float ct = m_known_state[it->first];
      float pbts = (it->second + consts::sig_smooth_factor * (m_unknown_state[it->first] / m_unknown) ) / (cs + consts::sig_smooth_factor);
      float pbwt = std::log(pbts / ct);
      if (pbwt > -100.0f)
        *out++ = std::make_pair(it->first, pbwt);
    }
  }

public:

  lexicon(const state_list & states) : m_states(states), m_known(0), m_unknown(0) {}

  lexicon(const state_list & states,
          const std::string & word_path,
          const std::string & sig_path,
          const std::string & word_state_path,
          const std::string & sig_state_path)
  : m_states(states), m_known(0), m_unknown(0)
  {
    std::ifstream word_in(word_path.c_str());
    std::ifstream sig_in(sig_path.c_str());
    std::ifstream word_state_in(word_state_path.c_str());
    std::ifstream sig_state_in(sig_state_path.c_str());
    load(word_in, sig_in, word_state_in, sig_state_in);
  }

  void load(std::istream & word_in,
            std::istream & sig_in,
            std::istream & word_state_in,
            std::istream & sig_state_in)
  {
    // get open class states
    // these are the set of states that have proven their arbitraryness enough
    // in training that we should consider them as candidates for signature
    // scoring
    m_open_class.clear();
    for (state_t i = 0; i != m_states.size(); ++i)
    {
      if (m_states[i].open_class)
        m_open_class.push_back(std::make_pair(i, 0));
    }

    // get words
    word_t max_word = 0;
    {
      std::string word;
      word_t index;
      m_word_index.clear();
      while (word_in >> index)
      {
        word_in.get(); // skip sp
        std::getline(word_in, word);
        m_word_index[word] = index;
        if (index > max_word)
          max_word = index;
      }
    }
    // get sigs
    word_t max_sig = 0;
    {
      std::string sig;
      word_t index;
      m_sig_index.clear();
      while (sig_in >> index)
      {
        sig_in.get(); // skip sp
        std::getline(sig_in, sig);
        m_sig_index[sig] = index;
        if (index > max_sig)
          max_sig = index;
      }
    }
    // get state|word
    {
      word_t word;
      std::pair< state_t, count_t > state_count;
      m_word.clear(); m_word.resize(max_word + 1);
      m_word_state.clear(); m_word_state.resize(max_word + 1);
      m_known_state.clear(); m_known_state.resize(m_states.size());
      while (word_state_in >> word >> state_count.first >> state_count.second)
      {
        m_word[word] += state_count.second;
        m_word_state[word].push_back(state_count);
        m_known_state[state_count.first] += state_count.second;
        m_known += state_count.second;
      }
    }
    // get state|sig
    {
      word_t sig;
      std::pair< state_t, count_t > state_count;
      m_sig.clear(); m_sig.resize(max_sig + 1);
      m_sig_state.clear(); m_sig_state.resize(max_sig + 1);
      m_unknown_state.clear(); m_unknown_state.resize(m_states.size());
      while (sig_state_in >> sig >> state_count.first >> state_count.second)
      {
        m_sig[sig] += state_count.second;
        m_sig_state[sig].push_back(state_count);
        m_unknown_state[state_count.first] += state_count.second;
        m_unknown += state_count.second;
      }
    }
    // fill m_any
    m_any.clear();
    for (state_t i = 0; i != m_states.size(); ++i)
    {
      if (m_known_state[i] > 0)
        m_any.push_back(std::make_pair(i, 0));
    }
    // sort our state counts by state
    std::sort(m_open_class.begin(), m_open_class.end(), cmp_state_t_count_t());
    for (std::vector< std::vector< std::pair< state_t, count_t > > >::iterator it = m_word_state.begin(); it != m_word_state.end(); ++it)
      std::sort(it->begin(), it->end(), cmp_state_t_count_t());
    for (std::vector< std::vector< std::pair< state_t, count_t > > >::iterator it = m_sig_state.begin(); it != m_sig_state.end(); ++it)
      std::sort(it->begin(), it->end(), cmp_state_t_count_t());
  }

  template <class OutputIterator>
  void score(const std::string & word, OutputIterator out, int pos = -1)
  {
    boost::unordered_map< std::string, word_t >::iterator it_w = m_word_index.find(word);
    if (it_w != m_word_index.end() && m_word[it_w->second] > 0) // have we seen this word?
      word_score(it_w->second, out, m_word[it_w->second] <= consts::smooth_threshold);
    else
      sig_score(word, out, pos);
  }
};

}}} // com::wavii::pfp

#endif // __LEXICON_HPP__
