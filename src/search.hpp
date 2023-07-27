#ifndef SEARCH_INCL
#define SEARCH_INCL
#include "evaluate.hpp"

centipawn negamax_search(Position& pos, uint8_t depth, centipawn alpha = Engine::NEGATIVE_INF_EVAL,
                         centipawn beta = Engine::POSITIVE_INF_EVAL);
#endif // SEARCH_INCL