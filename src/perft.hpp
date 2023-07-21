#ifndef PERFT_INCL
#define PERFT_INCL

#include "position.hpp"

namespace Engine
{

void perft_report(Position& pos, int depth);

void perft_report_divided(Position& pos, int depth);

} // namespace Engine

#endif // PERFT_INCL
