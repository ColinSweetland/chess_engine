#ifndef SEARCH_INCL
#define SEARCH_INCL
#include "evaluate.hpp"

scored_move alpha_beta_search(Position& pos, uint8_t depth, centipawn alpha = INT32_MIN, centipawn beta = INT32_MAX);
#endif // SEARCH_INCL