#include <stdio.h>
#include <math.h>

#include "game_state.h"
#include "bitboard.h"

int main()
{
    // gs = gs_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");

    dbg_print_gamestate(&gs);

    char* FEN = FEN_from_gs(&gs);

    puts(FEN);

    return 0;
}