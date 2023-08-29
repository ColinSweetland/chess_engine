#ifndef SEARCH_INCL
#define SEARCH_INCL
#include "chessmove.hpp"
#include "evaluate.hpp"

using Engine::centipawn;

namespace Search
{

struct search_info
{
    ChessMove best_move{};

    uint32_t nodes_searched = 0;

    centipawn score = Engine::NEGATIVE_INF_EVAL;
};

search_info negamax_root(Position& pos, int depth);

} // namespace Search
#endif // SEARCH_INCL