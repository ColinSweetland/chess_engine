#ifndef ENGINE_INCL
#define ENGINE_INCL

#include "position.h"

uint64_t perft(Position& p, int depth);

void perft_report(Position& p, int depth);

#endif // ENGINE_INCL