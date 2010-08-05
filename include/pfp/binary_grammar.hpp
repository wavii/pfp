#ifndef __BINARY_GRAMMAR_HPP__
#define __BINARY_GRAMMAR_HPP__

#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

#include <boost/unordered_map.hpp>

#include <pfp/util.hpp>
#include <pfp/state_list.hpp>

namespace com { namespace wavii { namespace pfp {

// binary rules have a left child, right child, parent, weight
// such as: VBD...PP => VP, 100
// weights represent the preference for their respective rule
class binary_grammar
{
public:

  struct relationship
  {
    state_t left;
    state_t rite;
    state_score_t result;
    relationship() {}
    relationship(state_t left_, state_t rite_, state_score_t result_) : left(left_), rite(rite_), result(result_) {}
  };

  struct cmp_left
  {
    bool operator()( const relationship & lhs, const relationship & rhs ) const { return lhs.left < rhs.left; }
  };

  struct cmp_rite
  {
    bool operator()( const relationship & lhs, const relationship & rhs ) const { return lhs.rite < rhs.rite; }
  };

  typedef std::vector< relationship >::const_iterator const_iterator;

private:

  static const state_t boundary = 412;
  const state_list &                                               m_states;
  std::vector< std::vector< relationship > >                       m_rules;
  std::vector< std::vector< relationship > >                       m_rules_parent;
  std::vector< relationship >                                      m_boundary_rules;

public:

  binary_grammar(const state_list & states) : m_states(states) {}

  binary_grammar(const state_list & states, const std::string & path)
  : m_states(states)
  {
    std::ifstream in(path.c_str());
    load(in);
  }

  void load(std::istream & in)
  {
    m_rules.clear(); m_rules.resize(m_states.size());
    m_rules_parent.clear(); m_rules_parent.resize(m_states.size());
    m_boundary_rules.clear();
    // stream is => [ left right parent score ]
    std::vector< size_t > sizes(m_states.size(), 0);
    std::vector< size_t > sizes_parent(m_states.size(), 0);
    std::vector< relationship > relationships;
    size_t boundary_size = 0;
    float f;
    relationship rel;
    // we want the potential space used here to be as tight as possible
    // to increase our cacheability.  so precompute all our sizes up front
    // so that we avoid resizing dynamically
    while (in >> rel.left >> rel.rite >> rel.result.state >> f)
    {
      rel.result.score = static_cast<score_t>(f * consts::score_resolution);
      relationships.push_back(rel);
      if (!m_states[rel.left].synthetic && !m_states[rel.rite].synthetic)
        ++boundary_size;
      else
        ++sizes[rel.left], ++sizes_parent[rel.result.state];
    }
    for (state_t i = 0; i != m_states.size(); ++i)
    {
      if (sizes[i] > 0)
        m_rules[i].reserve(sizes[i]);
      if (sizes_parent[i] > 0)
        m_rules_parent.reserve(sizes_parent[i]);
    }
    // boundary rules are a small set of rules that result in only a few top-level states
    // (such as S, S-FRAG, and so on) being able to combine with the boundary symbol to produce the goal state.
    m_boundary_rules.reserve(boundary_size);
    for (std::vector<relationship>::const_iterator it = relationships.begin(); it != relationships.end(); ++it)
    {
      if (!m_states[it->left].synthetic && !m_states[it->rite].synthetic)
        m_boundary_rules.push_back(*it);
      else
      {
        m_rules[it->left].push_back(*it);
        m_rules_parent[it->result.state].push_back(*it);
      }
    }
    // sort our rules for better cache hitting when operating on them
    for (state_t i = 0; i != m_states.size(); ++i)
    {
      std::sort(m_rules[i].begin(), m_rules[i].end(), cmp_rite());
      std::sort(m_rules_parent[i].begin(), m_rules_parent[i].end(), cmp_left());
    }
  }

  const std::vector< relationship > & get_rules(state_t left) const
  {
    return m_rules[left];
  }

  const std::vector< relationship > & get_rules_parent(state_t parent) const
  {
    return m_rules_parent[parent];
  }

  const_iterator boundary_begin() const
  {
    return m_boundary_rules.begin();
  }

  const_iterator boundary_end() const
  {
    return m_boundary_rules.end();
  }
};

}}} // com::wavii::pfp

#endif // __BINARY_GRAMMAR_HPP__
