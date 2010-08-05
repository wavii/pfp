#include <pfp/config.h>

using namespace com::wavii::pfp;

const char * consts::version = "0.1";
const float consts::score_resolution = 50.0f;
const score_t consts::empty_score = -32768;
const count_t consts::smooth_threshold = 100;
const float consts::word_smooth_factor = 0.2f;
const float consts::sig_smooth_factor = 1.0f;
const state_t consts::goal_state = 411;
const state_t consts::boundary_state = 412;
const score_t consts::epsilon = 0;
