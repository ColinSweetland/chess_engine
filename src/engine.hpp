#ifndef ENGINE_INCL
#define ENGINE_INCL

#include "position.hpp"

uint64_t perft(Position& pos, int depth);

void perft_report(Position& pos, int depth);

void perft_report_divided(Position& pos, int depth);

#endif // ENGINE_INCL