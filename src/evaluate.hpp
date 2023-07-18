#ifndef EVAL_INCL
#define EVAL_INCL

#include "chessmove.hpp"
#include "position.hpp"
#include "types.hpp"

#include <cstdint>

using centipawn = int32_t;

namespace Engine
{

// +/- 300 because we still need to prefer earlier depth checkmates
const centipawn WHITE_CHECKMATE_EVAL = INT32_MIN + 1000;
const centipawn BLACK_CHECKMATE_EVAL = INT32_MAX - 1000;

constexpr centipawn CHECKMATE_EVAL(COLOR c) { return c ? BLACK_CHECKMATE_EVAL : WHITE_CHECKMATE_EVAL; }

constexpr int EVAL_SIGN(COLOR c) { return c ? +1 : -1; }

// draw is equally bad for both sides
const centipawn DRAW_EVAL = 0;

centipawn evaluate(Position& pos, uint8_t depth = 0);

ChessMove best_move(Position& pos, uint8_t depth);

} // namespace Engine

#endif // EVAL_INCL