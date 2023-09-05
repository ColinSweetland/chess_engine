
#include "search.hpp"
#include "chessmove.hpp"
#include "evaluate.hpp"
#include "position.hpp"
#include "transposition.hpp"

#include <algorithm>
#include <cstdint>

using Engine::centipawn;
using namespace Search;

centipawn negamax_search(Position& pos, uint8_t depth, uint32_t& nodes_searched,
                         centipawn alpha = Engine::NEGATIVE_INF_EVAL, centipawn beta = Engine::POSITIVE_INF_EVAL);

// Finds the best move using search. Essentially a wrapper for the real negamax search,
// but needed because search returns an evaluation and we want a ChessMove
search_info Search::negamax_root(Position& pos, int depth)
{
    // We assume here that the position is not over (the engine wouldn't ask for a best move)

    assert(depth >= 1);

    search_info info = {};

    move_list psl = pos.pseudo_legal_moves();

    tt::entry entry = tt::lookup(pos.zhash());
    order_moves(psl, entry.best_move);

    for (ChessMove move : psl)
    {
        if (!pos.try_make_move(move))
            continue;

        centipawn move_eval =
            -negamax_search(pos, depth - 1, info.nodes_searched, Engine::NEGATIVE_INF_EVAL, -info.score);

        if (move_eval > info.score)
        {
            info.best_move = move;
            info.score     = move_eval;
        }

        pos.unmake_last();
    }

    assert(!info.best_move.is_null());

    return info;
}

// https://en.wikipedia.org/wiki/Negamax
centipawn negamax_search(Position& pos, uint8_t depth, uint32_t& nodes_searched, centipawn alpha, centipawn beta)
{
    // rep draw is a special case: always draw, we don't care about the tt or anything else
    if (pos.is_rep_draw())
        return Engine::DRAW_EVAL;

    // probe tt to see if we've seen this position
    tt::entry entry     = tt::lookup(pos.zhash());
    centipawn alphaOrig = alpha;

    // if the position has been seen, and we've
    //  searched below at least 'depth' amount
    if (tt::valid_entry(entry) && entry.depth >= depth)
    {
        if (entry.node_type == tt::NODE_TYPE::PV)
            return entry.value;

        // lowerbound
        else if (entry.node_type == tt::NODE_TYPE::CUT)
            alpha = std::max(alpha, entry.value);

        // upperbound
        else if (entry.node_type == tt::NODE_TYPE::ALL)
            beta = std::min(beta, entry.value);

        // cause cutoff if found
        if (alpha >= beta)
            return entry.value;
    }

    // generate pseudolegal moves,
    // we need to generate them to check if the position is checkmate
    move_list psl_moves = pos.pseudo_legal_moves();

    centipawn best_eval = Engine::NEGATIVE_INF_EVAL;
    ChessMove best_move = {};
    nodes_searched += 1;

    // note: this statement doesn't cover every game over case.
    // in many stalemate/checkmate cases, we will reach to the end of this function,
    // so don't assume the game isn't over now
    if (depth == 0 || pos.has_been_50_reversible_full_moves() || psl_moves.size() == 0)
    {
        best_eval = Engine::evaluate(pos, psl_moves);
        return best_eval;
    }

    // order moves to create earlier cutoffs
    order_moves(psl_moves, entry.best_move);

    for (ChessMove move : psl_moves)
    {
        // skip illegal moves
        if (!pos.try_make_move(move))
            continue;

        centipawn node_eval = -negamax_search(pos, depth - 1, nodes_searched, -beta, -alpha);

        // the move we are currently searching is the new best
        if (node_eval >= best_eval)
        {
            best_move = move;
            best_eval = node_eval;
        }

        // unmake move
        pos.unmake_last();

        // if the best eval becomes better than alpha, it is the new best globally
        if (best_eval >= alpha)
        {
            alpha     = best_eval;
            best_move = move;
        }

        // cause cutoff, move proven worse than other alternatives
        if (alpha >= beta)
            break;
    }

    // If node_eval is still negative infinite, no legal move was found.
    // the node is checkmate if the position is check, otherwise it's stalemate.

    if (best_eval == Engine::NEGATIVE_INF_EVAL)
    {
        // NOTE: don't use evaluate function here because it has to check for checkmate, it does
        // this by searching for a legal move. we already know if their was or wasn't a legal move

        // also don't bother entering this node into the TT, since their are no child nodes, it won't save time.
        if (pos.is_check())
            return Engine::LOST_EVAL + Engine::tempo_penalty(depth);
        else
            return Engine::DRAW_EVAL + Engine::tempo_penalty(depth);
    }

    // Now: store tt entry and return

    entry.full_hash = pos.zhash();
    entry.value     = best_eval;
    entry.best_move = best_move;
    entry.depth     = depth;

    // the search didn't improve alpha, therefore all children were searched.
    if (best_eval <= alphaOrig)
        entry.node_type = tt::NODE_TYPE::ALL;

    // the search improved alpha enough to cause a cutoff
    else if (best_eval >= beta)
        entry.node_type = tt::NODE_TYPE::CUT;

    // the search improved alpha, but didn't cause a cutoff. it is the PV at time of search
    else
        entry.node_type = tt::NODE_TYPE::PV;

    // store node info in the tranposition table
    tt::store(entry);

    return best_eval;
}
