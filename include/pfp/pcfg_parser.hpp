#ifndef __PCFG_PARSER_HPP__
#define __PCFG_PARSER_HPP__

#include <vector>
#include <exception>
#include <limits>
#include <boost/shared_ptr.hpp>

#include <pfp/util.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/unary_grammar.hpp>
#include <pfp/state_list.hpp>

namespace com { namespace wavii { namespace pfp {

// exhaustive parser for a probabilistic context-free grammar
// exploits dynamic programming to iteratively score every
// possible state for every possible span of tags, in a bottom-up
// approach.  optimized to visit the fewest combinations possible
// of a potentially n^3 * s^2 space: begin, end, split, left-child, right-child
class pcfg_parser
{
private:

  const state_list &    m_states; // list of states and their properties
  const unary_grammar &  m_ug;     // our unary grammar rules
  const binary_grammar & m_bg;     // our binary grammar rules

  void best_parse(node & tree, const std::vector< std::vector< state_score_t > > & sentence, workspace & ws, pos_t begin, pos_t end)
  {
    tree.score = ws.get(begin, end, tree.state);

    if (end - begin == 1)
    {
      // are we perhaps at a terminal state?
      std::vector< state_score_t >::const_iterator it_ts = std::lower_bound(sentence[begin].begin(), sentence[begin].end(), state_score_t(tree.state, 0));
      if (it_ts != sentence[begin].end() && it_ts->state == tree.state)
        return;
    }

    // first check binary rules
    binary_grammar::const_iterator it_br, beg_br, end_br;
    score_t split;
    if (begin == 0 && end == sentence.size()) // ah, the boundary symbol rules
      beg_br = m_bg.boundary_begin(), end_br = m_bg.boundary_end(), split = end - 1;
    else
      beg_br = m_bg.get_rules_parent(tree.state).begin(), end_br = m_bg.get_rules_parent(tree.state).end(), split = begin + 1;
    for (; split != end; ++split)
    {
      for (it_br = beg_br; it_br != end_br; ++it_br)
      {
        if (std::abs(it_br->result.score + ws.get(begin, split, it_br->left) + ws.get(split, end, it_br->rite) - tree.score) <= consts::epsilon)
        {
          tree.children.push_back(boost::shared_ptr<node>(new node(it_br->left, 0)));
          tree.children.push_back(boost::shared_ptr<node>(new node(it_br->rite, 0)));
          best_parse(*tree.children[0], sentence, ws, begin, split);
          best_parse(*tree.children[1], sentence, ws, split, end);
          return;
        }
      }
    }
    // now check unary rules, with non-closed grammar
    unary_grammar::const_iterator it_ur = m_ug.get_rules_parent(tree.state).begin(), end_ur = m_ug.get_rules_parent(tree.state).end();
    for (; it_ur != end_ur; ++it_ur)
    {
      if (std::abs(it_ur->result.score + ws.get(begin, end, it_ur->child) - tree.score) <= consts::epsilon)
      {
        tree.children.push_back(boost::shared_ptr<node>(new node(it_ur->child, 0)));
        best_parse(*tree.children[0], sentence, ws, begin, end);
        return;
      }
    }
    // kill screen!
    throw std::runtime_error("game over, man!");
  }

  void debinarize(node & tree)
  {
    // our grammar produces only binary and unary relations, and represents
    // n-ary relationships using synthetic states.  this method removes internal synthetic states
    // to convert 1-2-ary tree to n-ary
    std::vector< boost::shared_ptr< node > > children;
    for (std::vector< boost::shared_ptr< node > >::iterator it = tree.children.begin(); it != tree.children.end(); ++it)
    {
      debinarize(**it);
      if (m_states[(*it)->state].synthetic)
        children.insert(children.end(), (*it)->children.begin(), (*it)->children.end());
      else
        children.push_back( *it );
    }
    tree.children = children;
  }

public:

  pcfg_parser(const state_list & states, const unary_grammar & ug, const binary_grammar & bg)
  : m_states(states), m_ug(ug), m_bg(bg)
  {
  }

  // return true if a parse was found, and populate result tree
  // sentence word clouds must be sorted by state
  // workspace must be of adequate size for sentence length
  bool parse( const std::vector< std::vector< state_score_t > > & sentence,
              workspace & ws,
              node & tree )
  {
    pos_t sentence_size = static_cast<pos_t>(sentence.size());
    if ( sentence_size > ws.words )
      throw std::runtime_error("sentence too large for provided workspace");

    // initialize our workspace
    ws.clear(sentence_size);
    for (size_t i = 0; i != sentence.size(); ++i)
      ws.put(i, i + 1, sentence[i].begin(), sentence[i].end()); // provide the initial state from the sentence

    // hokay!  look inside ever-widening ranges for subranges that match unary/binary rules
    pos_t rsize, rbegin, rend, rsplit, rsplit_end;
    binary_grammar::const_iterator it_r, end_r;
    unary_grammar::const_iterator it_ur, end_ur;
    size_t i_ne, sz_ne;
    state_t left;
    int val, result;
    for (rsize = 1; rsize != sentence_size; ++rsize)
    {
      for (rbegin = 0, rend = rbegin + rsize; rend != sentence_size; ++rend, ++rbegin)
      {
        if (rsize > 1)
        {
          // first do binary rules
          // check states that have narrow extents that potentially leave space for a child after
          for (i_ne = 0, sz_ne = ws.seen_states[rbegin].size(); i_ne != sz_ne; ++i_ne)
          {
            left = ws.seen_states[rbegin][i_ne];
            // check the rite children
            bounds & br = ws.rite_extents[rbegin][left];
            for (it_r = m_bg.get_rules(left).begin(), end_r = m_bg.get_rules(left).end(); it_r != end_r; ++it_r)
            {
              bounds & bl = ws.left_extents[rend][it_r->rite];
              // do these left extents potentially leave space AND potentially reach far enough?
              if (bl.narrow < br.narrow || bl.wide > br.wide)
                continue;
              // okay, search a split from the earliest one could begin to the latest
              rsplit = std::max(br.narrow, bl.wide);
              rsplit_end = std::min(br.wide, bl.narrow);
              result = consts::empty_score;
              for (; rsplit <= rsplit_end; ++rsplit)
              {
                val = ws.get(rbegin, rsplit, left) + ws.get(rsplit, rend, it_r->rite);
                if (val > result)
                  result = val;
              }
              if (result != consts::empty_score)
                ws.put(rbegin, rend, it_r->result.state, it_r->result.score + result);
            }
          } // binary rules
        }
        // now do unary rules
        for ( it_ur = m_ug.closed_begin(), end_ur = m_ug.closed_end(); it_ur != end_ur; ++it_ur)
        {
          result = ws.get(rbegin, rend, it_ur->child);
          if (result != consts::empty_score)
            ws.put(rbegin, rend, it_ur->result.state, result + it_ur->result.score);
        } // unary rules
      } // rbegin
    } // rsize

    // run the boundary-symbol rules
    rbegin = 0;
    rend = rsize;
    rsplit = rsize - 1;
    score_t left_score, boundary_score = ws.get(rsplit, rend, consts::boundary_state);
    for (it_r = m_bg.boundary_begin(), end_r = m_bg.boundary_end(); it_r != end_r; ++it_r)
    {
      left_score = ws.get(rbegin, rsplit, it_r->left);
      if (left_score != consts::empty_score)
        ws.put(rbegin, rend, it_r->result.state, left_score + boundary_score + it_r->result.score);
    }

    if (ws.get(rbegin, rend, consts::goal_state) != consts::empty_score)
    {
      tree.state = consts::goal_state;
      best_parse(tree, sentence, ws, rbegin, rend);
      debinarize(tree);
      return true;
    }

    return false;
  }

  // return true if a parse was found, and populate result
  bool parse( const std::vector< std::vector< state_score_t > > & sentence,
              node & tree )
  {
    workspace ws(sentence.size(), m_states.size() );
    return parse(sentence, ws, tree);
  }

};

}}} // com::wavii::pfp

#endif // __PCFG_PARSER_HPP__
