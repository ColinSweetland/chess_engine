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
    if (depth == 0 || pos.is_game_over())
    {
        centipawn eval = Engine::evaluate(pos, depth);

        return std::make_pair(pos.last_move(), eval);
    }
    move_list ml = pos.pseudo_legal_moves();

    // maximizing player
    if (pos.side_to_move() == WHITE)
    {
        centipawn max_eval = INT32_MIN;
        ChessMove max_eval_move;

        for (ChessMove move : ml)
        {
            // skip illegal moves
            if (!pos.try_make_move(move))
                continue;

            scored_move scored = alpha_beta_search(pos, depth - 1, alpha, beta);

            // we found a new best move
            if (scored.second > max_eval)
            {
                max_eval_move = move;
                max_eval      = scored.second;
            }

            alpha = std::max(alpha, scored.second);

            // cause cutoff, opponent would never make this move
            if (beta <= alpha)
            {
                pos.unmake_last();
                break;
            }
            // unmake move from start of loop
            pos.unmake_last();
        }

        return std::make_pair(max_eval_move, max_eval);
    }
    // minimizing player, see comments above
    else
    {
        int32_t   min_eval = INT32_MAX;
        ChessMove min_eval_move;

        for (ChessMove move : ml)
        {
            if (!pos.try_make_move(move))
                continue;

            scored_move scored = alpha_beta_search(pos, depth - 1, alpha, beta);

            if (scored.second < min_eval)
            {
                min_eval_move = move;
                min_eval      = scored.second;
            }

            beta = std::min(beta, scored.second);

            if (beta <= alpha)
            {
                pos.unmake_last();
                break;
            }

            pos.unmake_last();
        }

        return std::make_pair(min_eval_move, min_eval);
    }
}

// game over eval: 0 if game isn't over
// else checkmate or draw value if it is
static centipawn game_over_eval(Position& pos)
{
    switch (pos.is_game_over())
    {
    case NOT_GAME_OVER:
        return 0;
        break;

    case FIFTY_MOVE_RULE:
    case STALEMATE:
        return Engine::DRAW_EVAL;
        break;

    case CHECKMATE:
        return Engine::CHECKMATE_EVAL(pos.side_to_move());
        break;
    }
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

centipawn Engine::evaluate(Position& pos, uint8_t depth)
{
    centipawn eval = game_over_eval(pos);

    // not mate
    if (eval == 0)
    {
        eval += piece_value_eval(pos);

        eval += piece_sq_table_eval(pos);
    }

    // tempo bonus means reaching a position with equivalent
    // evaluation one move earlier is equivalent to +5 centipawn
    centipawn tempo_bonus = EVAL_SIGN(pos.side_to_move()) * depth * 5;

    return eval + tempo_bonus;
}
