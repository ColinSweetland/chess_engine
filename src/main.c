#include "game_state.h"


int main()
{
    game_state gs = gs_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    dbg_print_gamestate(gs);

    return 0;
}