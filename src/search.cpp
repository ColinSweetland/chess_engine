
#include "chessmove.hpp"
#include "evaluate.hpp"
#include "position.hpp"

// Alpha beta search
scored_move alpha_beta_search(Position& pos, uint8_t depth, centipawn alpha = INT32_MIN, centipawn beta = INT32_MAX)
{
    // generate pseudolegal moves upfront,
    // we need to generate them in any case to check if the position is checkmate
    move_list psl_moves = pos.pseudo_legal_moves();

    // note: this statement doesn't cover every game over case.
    // in many stalemate/checkmate cases, we will reach to the end of this function,
    // so don't assume the game isn't over now
    if (depth == 0 || pos.has_been_50_reversible_full_moves() || psl_moves.size() == 0)
    {
        centipawn eval = Engine::evaluate(pos, psl_moves, depth);

        return std::make_pair(pos.last_move(), eval);
    }

    // order moves to create earlier cutoffs
    order_moves(psl_moves);

    // best_move is initialized to NULL MOVE
    // if we get to the end and it's still NULL MOVE: it was checkmate or stalemate
    ChessMove best_move = {};
    centipawn best_eval;

    // maximizing player
    if (pos.side_to_move() == WHITE)
    {
        best_eval = INT32_MIN;

        for (ChessMove move : psl_moves)
        {
            // skip illegal moves
            if (!pos.try_make_move(move))
                continue;

            scored_move scored = alpha_beta_search(pos, depth - 1, alpha, beta);

            // unmake move
            pos.unmake_last();

            // we found a new best move
            if (scored.second > best_eval)
            {
                best_move = move;
                best_eval = scored.second;
            }

            alpha = std::max(alpha, scored.second);

            // cause cutoff, opponent would never make this move
            if (beta <= alpha)
                break;
        }
    }
    // minimizing player, see comments above
    else
    {
        best_eval = INT32_MAX;

        for (ChessMove move : psl_moves)
        {
            if (!pos.try_make_move(move))
                continue;

            scored_move scored = alpha_beta_search(pos, depth - 1, alpha, beta);

            pos.unmake_last();

            if (scored.second < best_eval)
            {
                best_move = move;
                best_eval = scored.second;
            }

            beta = std::min(beta, scored.second);

            if (beta <= alpha)
                break;
        }
    }

    // NOTE: don't use evaluate function here because it has to check for checkmate, it does
    // this by searching for a legal move. we already know if their was or wasn't a legal move

    // we found some legal, best move -> return
    if (!best_move.is_null())
        return std::make_pair(best_move, best_eval);

    // their were no legal moves and check -> checkmate
    else if (pos.is_check())
        return std::make_pair(pos.last_move(), Engine::CHECKMATE_EVAL(pos.side_to_move()));

    // no legal moves and not check -> stalemate
    else
        return std::make_pair(pos.last_move(), Engine::DRAW_EVAL);
}
