#ifndef EVAL_INCL
#define EVAL_INCL

#include "chessmove.hpp"
#include "position.hpp"
#include "types.hpp"

#include <cstdint>

using centipawn = int32_t;

namespace Engine
{

// +/- some because we still need to prefer earlier depth checkmates
constexpr centipawn WHITE_CHECKMATE_EVAL = INT32_MIN + 1000;
constexpr centipawn BLACK_CHECKMATE_EVAL = INT32_MAX - 1000;

constexpr centipawn CHECKMATE_EVAL(COLOR mated) { return mated == BLACK ? BLACK_CHECKMATE_EVAL : WHITE_CHECKMATE_EVAL; }

constexpr int EVAL_SIGN(COLOR c) { return c ? +1 : -1; }

// draw is equally bad for both sides
constexpr centipawn DRAW_EVAL = 0;

centipawn evaluate(Position& pos, move_list& pseudo_legal_moves, uint8_t depth = 0);

ChessMove best_move(Position& pos, uint8_t depth);

} // namespace Engine

#endif // EVAL_INCL