
#include "search.hpp"
#include "evaluate.hpp"
#include "position.hpp"

#include <cstdint>

using Engine::centipawn;

// https://en.wikipedia.org/wiki/Negamax
centipawn negamax_search(Position& pos, uint8_t depth, uint32_t& nodes_searched, centipawn alpha, centipawn beta)
{
    // generate pseudolegal moves upfront,
    // we need to generate them in any case to check if the position is checkmate
    move_list psl_moves = pos.pseudo_legal_moves();

    centipawn node_eval = Engine::NEGATIVE_INF_EVAL;
    nodes_searched += 1;
    // note: this statement doesn't cover every game over case.
    // in many stalemate/checkmate cases, we will reach to the end of this function,
    // so don't assume the game isn't over now
    if (depth == 0 || pos.has_been_50_reversible_full_moves() || psl_moves.size() == 0)
    {
        node_eval = Engine::evaluate(pos, psl_moves);
        return node_eval + ((node_eval > 0 ? Engine::tempo_bonus(depth) : Engine::tempo_penalty(depth)));
    }

    // order moves to create earlier cutoffs
    order_moves(psl_moves);

    for (ChessMove move : psl_moves)
    {
        // skip illegal moves
        if (!pos.try_make_move(move))
            continue;

        node_eval = std::max(node_eval, -negamax_search(pos, depth - 1, nodes_searched, -beta, -alpha));

        // unmake move
        pos.unmake_last();

        alpha = std::max(alpha, node_eval);

        // cause cutoff, opponent would never make this move
        if (alpha >= beta)
            break;
    }

    // NOTE: don't use evaluate function here because it has to check for checkmate, it does
    // this by searching for a legal move. we already know if their was or wasn't a legal move

    // we found some legal, best move -> return
    if (node_eval != Engine::NEGATIVE_INF_EVAL)
        return node_eval + Engine::tempo_bonus(depth);

    // their were no legal moves and check -> checkmate
    else if (pos.is_check())
        return Engine::LOST_EVAL + Engine::tempo_penalty(depth);

    // no legal moves and not check -> stalemate
    else
        return Engine::DRAW_EVAL + Engine::tempo_penalty(depth);
}
