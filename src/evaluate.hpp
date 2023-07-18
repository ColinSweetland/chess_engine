#ifndef EVAL_INCL
#define EVAL_INCL

#include "chessmove.hpp"
#include "position.hpp"

#include <cstdint>

using centipawn = int32_t;

namespace Engine
{

const centipawn CHECKMATE_EVAL[2] = {INT32_MIN, INT32_MAX};

centipawn evaluate(Position& pos);

ChessMove best_move(Position& pos, uint8_t depth);

} // namespace Engine

#endif // EVAL_INCL