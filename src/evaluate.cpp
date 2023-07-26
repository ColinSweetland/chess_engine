#include "evaluate.hpp"
#include "bitboard.hpp"
#include "chessmove.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"

#include <cstdint>

// Alpha beta search
static scored_move alpha_beta_search(Position& pos, uint8_t depth, centipawn alpha = INT32_MIN,
                                     centipawn beta = INT32_MAX)
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

static centipawn piece_value_eval(Position& pos)
{
    static std::array<centipawn, KING + 1> piece_to_cp_score{0,         0,         100 /*P*/, 300 /*N*/,
                                                             320 /*B*/, 500 /*R*/, 900 /*Q*/};

    centipawn eval = 0;

    for (int p = PAWN; p < KING; p++)
    {
        // white (positive) piece score
        eval += piece_to_cp_score[p] * BB_POPCNT(pos.pieces(WHITE, static_cast<PIECE>(p)));
        // black (negative) piece score
        eval -= piece_to_cp_score[p] * BB_POPCNT(pos.pieces(BLACK, static_cast<PIECE>(p)));
    }

    return eval;
}

// clang-format off

static centipawn piece_sq_tables[6][64] = {
    /* PAWNS */
    {
        0  ,  0 ,   0 ,  0 ,   0 ,   0 ,  0 ,  0 ,
        50 , 50 ,  50 , 50 ,  50 ,  50 , 50 , 50 ,
        10 , 10 ,  20 , 30 ,  30 ,  20 , 10 , 10 ,
        5  ,  5 ,  10 , 25 ,  25 ,  10 ,  5 ,  5 ,
        0  ,  0 ,   0 , 20 ,  20 ,   0 ,  0 ,  0 ,
        5  , -5 , -10 ,  0 ,   0 , -10 , -5 ,  5 ,
        5  , 10 ,  10 ,-20 , -20 ,  10 ,  10,  5 ,
        0  ,  0 ,   0 ,  0 ,   0 ,   0 ,  0 ,  0
    },
    /* KNIGHTS */
    {
        -50 , -40 , -30 , -30 , -30 , -30 , -40, -50,
        -40 , -20 ,   0 ,   0 ,   0 ,   0 , -20, -40,
        -30 ,   0 ,  10 ,  15 ,  15 ,  10 ,   0, -30,
        -30 ,   5 ,  15 ,  20 ,  20 ,  15 ,   5, -30,
        -30 ,   0 ,  15 ,  20 ,  20 ,  15 ,   0, -30,
        -30 ,   5 ,  10 ,  15 ,  15 ,  10 ,   5, -30,
        -40 , -20 ,   0 ,   5 ,   5 ,   0 , -20, -40,
        -50 , -40 , -30 , -30 , -30 , -30 , -40, -50,
    },
    /* BISHOPS */
    {
        -20 , -10 , -10 , -10 , -10 , -10 , -10 , -20 ,
        -10 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 , -10 ,
        -10 ,   0 ,   5 ,  10 ,  10 ,   5 ,   0 , -10 ,
        -10 ,   5 ,   5 ,  10 ,  10 ,   5 ,   5 , -10 ,
        -10 ,   0 ,  10 ,  10 ,  10 ,  10 ,   0 , -10 ,
        -10 ,  10 ,  10 ,  10 ,  10 ,  10 ,  10 , -10 ,
        -10 ,   5 ,   0 ,   0 ,   0 ,   0 ,   5 , -10 ,
        -20 , -10 , -10 , -10 , -10 , -10 , -10 , -20 ,
    },
    /* ROOK */
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    },
    /* QUEEN */
    {
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10,   0,   0,  0,  0,   0,   0, -10,
        -10,   0,   5,  5,  5,   5,   0, -10,
         -5,   0,   5,  5,  5,   5,   0,  -5,
          0,   0,   5,  5,  5,   5,   0,  -5,
        -10,   5,   5,  5,  5,   5,   0, -10,
        -10,   0,   5,  0,  0,   0,   0, -10,
        -20, -10, -10, -5, -5, -10, -10, -20
    },
    /* KING */
    {
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
         20,  20,   0,   0,   0,   0,  20,  20,
         20,  30,  10,   0,   0,  10,  30,  20
    }

};

// clang-format on

centipawn piece_sq_table_eval(Position& pos)
{
    centipawn eval = 0;

    for (square bs_sq = 0; bs_sq < 64; bs_sq++)
    {
        // since the psqt are indexed "upside down" we must mirror the white sq
        square ws_sq = MIRROR_VERT_SQ(bs_sq);

        PIECE ws_sq_piece = pos.piece_at_sq(ws_sq);
        PIECE bs_sq_piece = pos.piece_at_sq(bs_sq);

        // This isn't efficient, we call color_at_sq 2 times more than we need to

        if (ws_sq_piece != NO_PIECE && pos.color_at_sq(ws_sq) == WHITE)
            // we still index into the table using blackside sq so evaluations match
            eval += piece_sq_tables[ws_sq_piece - PAWN][bs_sq];

        if (bs_sq_piece != NO_PIECE && pos.color_at_sq(bs_sq) == BLACK)
            // since black is the minimizing player, we must subtract here
            eval -= piece_sq_tables[bs_sq_piece - PAWN][bs_sq];
    }

    return eval;
}

ChessMove Engine::best_move(Position& pos, uint8_t depth) { return alpha_beta_search(pos, depth).first; }

// we must take a movelist as a parameter to check if it's checkmate or stalemate
centipawn Engine::evaluate(Position& pos, move_list& pseudo_legal_moves, uint8_t depth)
{
    // check for a single legal move: to verify not checkmate or stalemate
    bool has_legal_move = false;

    for (auto move : pseudo_legal_moves)
    {
        if (pos.try_make_move(move))
        {
            has_legal_move = true;
            pos.unmake_last();
            break;
        }
    }

    // if we don't have legal moves, it's checkmate or stalemate
    if (!has_legal_move)
    {
        if (pos.is_check())
            return Engine::CHECKMATE_EVAL(pos.side_to_move());
        else
            return Engine::DRAW_EVAL;
    }

    // if we had a legal moves, but it's been 50 reversible full moves, it's a draw
    if (pos.has_been_50_reversible_full_moves())
    {
        return Engine::DRAW_EVAL;
    }

    // finally: evaluation for normal positions
    centipawn eval = piece_value_eval(pos) + piece_sq_table_eval(pos);



    // add tempo bonus : reaching a position with equivalent
    // evaluation one move earlier is equivalent to +5 centipawn
    eval += EVAL_SIGN(pos.side_to_move()) * depth * 5;

    return eval;
}
