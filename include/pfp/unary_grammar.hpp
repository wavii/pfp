#ifndef __UNARY_GRAMMAR_HPP__
#define __UNARY_GRAMMAR_HPP__

#include <vector>
#include <fstream>
#include <string>

#include <boost/unordered_set.hpp>
#include <pfp/util.hpp>
#include <pfp/state_list.hpp>

namespace com { namespace wavii { namespace pfp {

// unary rules have a child, parent, weight
// such as: NNP => NP, 100
// weights represent the preference for their respective rule
// we also try to produce the closure of the grammar by creating transitive associations:
// A->B, 100 and B->C, 100 : A->C 200
// but we only go one level deep - probably not a full closure
class unary_grammar
{
public:

  struct relationship
  {
    state_t child;
    state_score_t result;
    relationship() {}
    relationship(state_t child_, state_score_t result_) : child(child_), result(result_) {}
    bool operator < (const relationship & rhs) const
    {
      return child < rhs.child || (child == rhs.child && result.state < rhs.result.state);
    }
  };

  typedef std::vector< relationship >::const_iterator const_iterator;

private:

  const state_list &                         m_states;
  std::vector< std::vector< relationship > > m_rules_parent; // rules indexed by parent
  std::vector< relationship >                m_rules_closed; // rules' closure for transitive relationships

public:

  unary_grammar(const state_list & states) : m_states(states) {}

  unary_grammar(const state_list & states, const std::string & path)
  : m_states(states)
  {
    std::ifstream in(path.c_str());
    load(in);
  }

  void load(std::istream & in)
  {
    m_rules_parent.resize(m_states.size());
    m_rules_closed.clear();
    // stream is => [ child parent score ]
    relationship rel;
    // keep track of closed rels to avoid dupes
    std::vector< std::vector< relationship > > children_closed(m_states.size());
    std::vector< std::vector< relationship > > parents_closed(m_states.size());
    boost::unordered_set< std::pair< state_t, state_t > > closed_rels;
    // initialize unity rules in closure
    for (state_t i = 0; i != m_states.size(); ++i)
    {
      parents_closed[i].push_back(relationship(i, state_score_t(i, 0.0f)));
      children_closed[i].push_back(relationship(i, state_score_t(i, 0.0f)));
      closed_rels.insert(std::make_pair(i, i));
    }
    float f;
    size_t parents_closed_total = 0;
    while (in >> rel.child >> rel.result.state >> f)
    {
      rel.result.score = static_cast<score_t>(f * consts::score_resolution);
      parents_closed[rel.child].push_back(rel); ++parents_closed_total;
      children_closed[rel.result.state].push_back(rel);
      m_rules_parent[rel.result.state].push_back(rel);
      closed_rels.insert(std::make_pair(rel.child, rel.result.state));
      // add transitive rule: [all parents of rel.parent] -> [all children of rel.child]
      for (state_t i = 0, sz_i = parents_closed[rel.result.state].size(); i != sz_i; ++i)
      {
        relationship & reli = parents_closed[rel.result.state][i];
        for (state_t j = 0, sz_j = children_closed[rel.child].size(); j != sz_j; ++j)
        {
          relationship & relj = children_closed[rel.child][j];
          if (closed_rels.find(std::make_pair(relj.child, reli.result.state)) == closed_rels.end())
          {
            closed_rels.insert(std::make_pair(relj.child, reli.result.state));
            relationship trans_rel(relj.child, state_score_t(reli.result.state, reli.result.score + rel.result.score + relj.result.score));
            parents_closed[relj.child].push_back(trans_rel); ++parents_closed_total;
            children_closed[reli.result.state].push_back(trans_rel);
          }
        }
      }
    }
    m_rules_closed.reserve(parents_closed_total);
    // populate closed rules (all except for original identity rule)
    for (state_t i = 0; i != m_states.size(); ++i)
    {
      std::sort(parents_closed[i].begin() + 1, parents_closed[i].end());
      std::copy(parents_closed[i].begin() + 1, parents_closed[i].end(), std::back_inserter(m_rules_closed));
    }
  }

  const std::vector< relationship > & get_rules_parent(state_t parent) const
  {
    return m_rules_parent[parent];
  }

  const_iterator closed_begin() const
  {
    return m_rules_closed.begin();
  }

  const_iterator closed_end() const
  {
    return m_rules_closed.end();
  }
};

}}} // com::wavii::pfp

#endif // __UNARY_GRAMMAR_HPP__
