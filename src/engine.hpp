#ifndef ENGINE_INCL
#define ENGINE_INCL

#include "position.hpp"

namespace Engine
{

void uci_loop();

void perft_report(Position& pos, int depth);

void perft_report_divided(Position& pos, int depth, bool print_fen = false);

} // namespace Engine

#endif // ENGINE_INCL