#ifndef __STATE_LIST_HPP__
#define __STATE_LIST_HPP__

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include <pfp/util.hpp>

namespace com { namespace wavii { namespace pfp {

class state_list
{
public:

  struct state
  {
    std::string tag;
    state_t     index;
    bool        synthetic;   // state is for pcfg internal use
    bool        open_class;  // state is safe for lexicon sig. guessing
    bool operator < (const state & other) const
    {
      return index < other.index;
    }
    // removes functional modifiers of a state, markovizations, and other junk
    std::string basic_category()
    {
      const char delims[] = {'=', '|', '#', '^', '~', '_'};
      size_t i = 0;
      for (; i != tag.size(); ++i)
      {
        if (std::find(delims, delims + sizeof(delims), tag[i]) != delims + sizeof(delims))
          break;
      }
      return tag.substr(0, i);
    }
  };

  typedef std::vector<state>::iterator iterator;

  typedef std::vector<state>::const_iterator const_iterator;

private:

  state_t m_size;
  std::vector< state > m_states;

public:

  state_list() : m_size(0) {}

  state_list(const std::string & path)
  {
    std::ifstream in(path.c_str());
    load(in);
  }

  void load(std::istream & in)
  {
    state s;
    while (in >> s.index)
    {
      in.get(); // skip space
      std::getline(in, s.tag);
      s.synthetic = (s.tag[0] == '@');
      s.open_class = (s.tag[0] == '+');
      if (s.open_class)
        s.tag = s.tag.substr(1);
      m_states.push_back(s);
    }
    std::sort(m_states.begin(), m_states.end());
  }

  const state & operator[](int index) const { return m_states[index]; }

  state & operator[](int index) { return m_states[index]; }

  state_t size() const { return m_states.size(); }

  iterator begin() { return m_states.begin(); }

  iterator end() { return m_states.end(); }

  const_iterator begin() const { return m_states.begin(); }

  const_iterator end() const { return m_states.end(); }

};

}}} // com::wavii::pfp

#endif // __STATE_LIST_HPP__
