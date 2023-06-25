#ifndef ENGINE_INCL
#define ENGINE_INCL

#include "position.hpp"

void perft(Position& pos, int depth, std::vector<uint64_t>& perft_results);

void perft_report(Position& pos, int depth);

void perft_report_divided(Position& pos, int depth, bool print_fen = false);

#endif // ENGINE_INCL