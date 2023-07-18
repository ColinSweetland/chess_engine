#ifndef ENGINE_INCL
#define ENGINE_INCL

#include "chessmove.hpp"
#include "position.hpp"
#include "types.hpp"

namespace Engine
{

void uci_loop();

void perft_report(Position& pos, int depth);

void perft_report_divided(Position& pos, int depth);

} // namespace Engine

#endif // ENGINE_INCL