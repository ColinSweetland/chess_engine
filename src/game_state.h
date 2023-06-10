#ifndef GAMESTATE_INCL
#define GAMESTATE_INCL

#include "types.h"

game_state gs_from_FEN(const char *FEN);

char *FEN_from_gs(const game_state *gs);

PIECE piece_at_sq(const game_state *gs, int sq);

bool sq_attacked(const game_state *gs, int sq, COLOR attacking_color);

void print_gamestate(const game_state *gs);

void dbg_print_gamestate(const game_state *gs);

#endif // GAMESTATE_INCL
