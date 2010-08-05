#ifndef __CONFIG_H__
#define __CONFIG_H__

namespace com { namespace wavii { namespace pfp {

// typedefs and consts

typedef unsigned short state_t; // currently around 12,000 distinct states
typedef unsigned short word_t;  // around 47,000 words in our lexicon
typedef float count_t;          // counts in our lexicon (just keep float for easy manipulation)
typedef short score_t;          // we don't need full float resolution, use short for memory+speed gain
typedef unsigned char pos_t;    // word position in a sentence, never more than 100

struct consts
{
  static const char * version;           // version of pfp engine
  static const float score_resolution;   // spread scores more evenly across a downcast space
  static const score_t empty_score;      // hasn't been scored
  static const count_t smooth_threshold; // significance threshold for words we haven't seen enough to build our lexicon
  static const float word_smooth_factor; // add this much smoothing to word weights
  static const float sig_smooth_factor;  // add this much smoothing to signature weights
  static const state_t goal_state;       // our goal state, which we strive to attain
  static const state_t boundary_state;   // the boundary state marks the end of a sentence
  static const score_t epsilon;          // an almost-or-totally insignificant amount
};

}}} // com::wavi::pfp

#endif // __CONFIG_H__
