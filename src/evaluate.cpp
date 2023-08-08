#include "evaluate.hpp"
#include "chessmove.hpp"
#include "position.hpp"
#include "search.hpp"
#include "types/bitboard.hpp"

#include <cstdint>

using Engine::centipawn;

static centipawn piece_value_eval(Position& pos)
{
    centipawn eval = 0;

    COLOR stm = pos.side_to_move();

    for (int p = PAWN; p < KING; p++)
    {
        // side to move (positive) piece score
        eval += Engine::piece_to_cp_score(static_cast<PIECE>(p)) * popcnt(pos.pieces(stm, static_cast<PIECE>(p)));
        // enemy (negative) piece score
        eval -= Engine::piece_to_cp_score(static_cast<PIECE>(p)) * popcnt(pos.pieces(!stm, static_cast<PIECE>(p)));
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
        square ws_sq = mirror_vertically(bs_sq);

        PIECE ws_sq_piece = pos.piece_at_sq(ws_sq);
        PIECE bs_sq_piece = pos.piece_at_sq(bs_sq);

        // This isn't efficient, we call color_at_sq 2 times more than we need to
        if (ws_sq_piece != KING && pos.color_at_sq(ws_sq) == WHITE)
            // we still index into the table using blackside sq so evaluations match
            eval += piece_sq_tables[ws_sq_piece][bs_sq];

        if (bs_sq_piece != KING && pos.color_at_sq(bs_sq) == BLACK)
            eval -= piece_sq_tables[bs_sq_piece][bs_sq];
    }

    // presently, eval is pos for white, negative for black
    // if black, flip such that a positive position is good for black
    if (pos.side_to_move() == BLACK)
        eval = -eval;

    return eval;
}

// best move finds the best move using search. Essentially a wrapper for search,
// but needed because search returns an evaluation and we want a ChessMove
ChessMove Engine::best_move(Position& pos, int depth)
{
    // We assume here that the position is not over (the engine wouldn't ask for a best move)

    assert(depth >= 1);

    ChessMove best_move;
    centipawn best_evaluation = NEGATIVE_INF_EVAL;

    move_list psl = pos.pseudo_legal_moves();

    order_moves(psl);

    for (ChessMove move : psl)
    {
        if (!pos.try_make_move(move))
            continue;

        centipawn move_eval = -negamax_search(pos, depth - 1);

        if (move_eval > best_evaluation)
        {
            best_move       = move;
            best_evaluation = move_eval;
        }

        pos.unmake_last();
    }

    std::cout << "info depth " << depth << " score cp " << best_evaluation << "\n";
    assert(!best_move.is_null());
    return best_move;
}

// evaluate RELATIVE TO SIDE TO MOVE
centipawn Engine::evaluate(Position& pos, move_list& pseudo_legal_moves)
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
            return LOST_EVAL;

        else
            return DRAW_EVAL;
    }

    // if we had a legal moves, but it's been 50 reversible full moves, it's a draw
    else if (pos.has_been_50_reversible_full_moves())
        return Engine::DRAW_EVAL;

    // finally: evaluation for normal positions
    centipawn eval = piece_value_eval(pos) + piece_sq_table_eval(pos);

    return eval;
}
