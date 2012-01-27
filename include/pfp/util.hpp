#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <vector>
#include <limits>
#include <algorithm>
#include <boost/shared_ptr.hpp>

#include <pfp/config.h>

namespace com { namespace wavii { namespace pfp {

struct state_score_t
{
  state_t state;
  score_t score;

  state_score_t() : state(0), score(0) {}
  state_score_t(state_t state_, score_t score_) : state(state_), score(score_) {}

  bool operator < (const state_score_t & rhs) const
  {
    return state < rhs.state;
  }
};

// a node of an n-ary parse tree
struct node
{
  state_t           state;
  score_t           score;
  std::vector< boost::shared_ptr< node > > children;
  node() : state(0), score(0) {}
  node(state_t state_, score_t score_) : state(state_), score(score_) {}
};

// bounds represent the widest and the narrowest that we've ever seen
// a state extent right from a start position, or extend left from an
// end position
struct bounds
{
  bounds() : narrow(0), wide(0) {}
  bounds(pos_t narrow_, pos_t wide_) : narrow(narrow_), wide(wide_) {}
  pos_t narrow;
  pos_t wide;
};

struct workspace
{
  pos_t words;    // the longest sentence this workspace will support
  state_t states; // the number of states this workspace will support

  std::vector< std::vector< bounds > > left_extents;  // end, state => bounds
  std::vector< std::vector< bounds > > rite_extents; // begin, state => bounds
  std::vector< std::vector< state_t > > seen_states; // begin => state (sparse)
  std::vector< std::vector< std::vector < score_t > > > state_scores; // begin, end => states

  workspace(pos_t words_, state_t states_)
  : words(words_), states(states_),
  left_extents(words + 1), rite_extents(words),
  seen_states(words), state_scores(words)
  {
    for (pos_t i = 0; i != words + 1; ++i)
      left_extents[i].resize(states);
    for (pos_t i = 0; i != words; ++i)
      rite_extents[i].resize(states);
    for (pos_t i = 0; i != words; ++i)
      seen_states[i].reserve(1024);
    for (pos_t i = 0; i != words; ++i)
      state_scores[i].resize(words + 1);
    // note the upper triangularness: end > start
    // and the range to end is inclusive, so length + 1
    for (pos_t i = 0; i != words; ++i)
    {
      for (pos_t j = i + 1; j != words + 1; ++j)
        state_scores[i][j].resize(states);
    }
  }

  void clear()
  {
    clear(words);
  }

  // any workspace size >= our sentence size will do,
  // but we only need to clear up to as many words as we need
  void clear(pos_t sentence_size)
  {
    for (pos_t i = 0; i != sentence_size + 1; ++i)
      std::fill(left_extents[i].begin(), left_extents[i].end(), bounds(std::numeric_limits<pos_t>::min(), std::numeric_limits<pos_t>::max()));
    for (pos_t i = 0; i != sentence_size; ++i)
      std::fill(rite_extents[i].begin(), rite_extents[i].end(), bounds(std::numeric_limits<pos_t>::max(), std::numeric_limits<pos_t>::min()));
    for (pos_t i = 0; i != sentence_size; ++i)
      seen_states[i].clear();
    for (pos_t i = 0; i != sentence_size; ++i)
    {
      for (pos_t j = i + 1; j != sentence_size + 1; ++j)
        std::fill(state_scores[i][j].begin(), state_scores[i][j].end(), consts::empty_score);
    }
  }

  void put(pos_t begin, pos_t end, state_t state, score_t score)
  {
    score_t & f = state_scores[begin][end][state];
    if (f == consts::empty_score)
    {
      f = score;
      // this is the first time we've seen this state occupy [begin, end)
      // let's update our bounds information to fit
      bounds & bl = left_extents[end][state];
      bounds & br = rite_extents[begin][state];
      // sneaky!  begin can never be > bl.narrow because diff is always increasing
      // UNLESS we are in initial state.  mirror applies for extents below
      if (begin > bl.narrow)
        bl.narrow = bl.wide = begin;
      else if (begin < bl.wide)
        bl.wide = begin;
      if (end < br.narrow)
      {
        br.narrow = br.wide = end;
        seen_states[begin].push_back(state);
      }
      else if (end > br.wide)
        br.wide = end;
    }
    else if (f < score)
      f = score;
  }

  template<class InputIterator>
  void put(pos_t begin, pos_t end, InputIterator ss_begin, InputIterator ss_end)
  {
    for (; ss_begin != ss_end; ++ss_begin)
      put(begin, end, ss_begin->state, ss_begin->score);
  }

  score_t get(pos_t begin, pos_t end, state_t state)
  {
    return state_scores[begin][end][state];
  }
};

// stich a node tree to a an output
template<class Out, class InputIterator, class StateList>
InputIterator stitch(Out & out, const node & tree, InputIterator word_it, StateList & states)
{
  // don't output boundary
  if (tree.state == consts::boundary_state)
    return word_it;

  out << '(';
  if (states[tree.state].basic_category().empty() && tree.children.empty())
      out << *word_it;
  else
      out << states[tree.state].basic_category();
  out << ' ';

  if ( tree.children.empty() ) {
    out << *word_it++;
  }
  else
  {
    for (std::vector< boost::shared_ptr< node > >::const_iterator it = tree.children.begin(); it != tree.children.end(); ++it)
    {
      word_it = stitch(out, **it, word_it, states);
      if (it != tree.children.end() - 1)
        out << ' '; // give some space to lists
    }
  }
  out << ')';
  return word_it;
}

}}} // com::wavii::pfp

#endif // __UTIL_HPP__
