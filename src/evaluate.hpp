#ifndef EVAL_INCL
#define EVAL_INCL

#include "./types/pieces.hpp"
#include "chessmove.hpp"

#include <cstdint>

class ChessMove;
class Position;

namespace Engine
{

// unit for evaluation. 100 = value of a pawn.
using centipawn = int32_t;

constexpr centipawn NEGATIVE_INF_EVAL = (INT32_MIN + 5);
constexpr centipawn POSITIVE_INF_EVAL = (INT32_MAX - 5);

// +/- some because we still need to prefer earlier depth checkmates
constexpr centipawn LOST_EVAL = NEGATIVE_INF_EVAL + 10000;
constexpr centipawn WON_EVAL  = POSITIVE_INF_EVAL - 10000;

// draw is equally bad for both sides
constexpr centipawn DRAW_EVAL = 0;

constexpr centipawn piece_to_cp_score(PIECE p)
{
    //  NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN
    constexpr centipawn valuelookup[] = {0, 100, 310, 325, 500, 900};
    return valuelookup[p];
};

// tempo bonus: their should be a bonus for reaching a positive position at an earlier depth
// Note: earlier depth is the bigger number, so the implementation here IS correct
constexpr centipawn tempo_bonus(uint8_t depth) { return depth * 5; }

// likewise: avoid checkmate at earlier depths, even if it's guarunteed: could help draw on time
constexpr centipawn tempo_penalty(uint8_t depth) { return -tempo_bonus(depth); }

// full evaluation of the position, relative to side moving (needed for negamax search).
// move_list param helps us to see if position is checkmate
// tempo bonus/penalty NOT included, must be added if desired
centipawn evaluate(Position& pos, move_list& pseudo_legal_moves);

} // namespace Engine

#endif // EVAL_INCL