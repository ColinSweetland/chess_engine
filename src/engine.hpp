#ifndef ENGINE_INCL
#define ENGINE_INCL

#include "chessmove.hpp"
#include "position.hpp"
#include "types.hpp"

namespace Engine
{

ChessMove best_move(Position pos);

void uci_loop();

void perft_report(Position& pos, int depth);

void perft_report_divided(Position& pos, int depth, bool print_fen = false);

} // namespace Engine

#endif // ENGINE_INCL