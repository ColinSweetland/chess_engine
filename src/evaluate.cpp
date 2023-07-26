#include "evaluate.hpp"
#include "bitboard.hpp"
#include "chessmove.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "search.hpp"
#include "types.hpp"

#include <cstdint>

static centipawn piece_value_eval(Position& pos)
{
    centipawn eval = 0;

    for (int p = PAWN; p < KING; p++)
    {
        // white (positive) piece score
        eval += Engine::piece_to_cp_score(static_cast<PIECE>(p)) * BB_POPCNT(pos.pieces(WHITE, static_cast<PIECE>(p)));
        // black (negative) piece score
        eval -= Engine::piece_to_cp_score(static_cast<PIECE>(p)) * BB_POPCNT(pos.pieces(BLACK, static_cast<PIECE>(p)));
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
