#include <stdio.h>
#include <stdlib.h>

#include "game_state.h"
#include "movegen.h"
#include "bitboard.h"

/*
void print_moves(move_list_node *move)
{
    for(move_list_node *m = move; m->next != NULL; m = m->next)
    {
        printf("From: %d To: %d\n",m->move.from_square, m->move.to_square);
    }
}
*/

int main()
{
    //game_state gs = gs_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/5K2/P1PP1PPP/RNBQ1B1R w KQkq - 1 2");
    game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/pppppppp/P1PP1PPP/RNBQKB1R w KQkq - 1 2");

    print_gamestate(&gs);
    
    bitboard k = bb_pawn_attacks_w(&gs, WHITE);

    print_bb(k);
  

    printf("Exiting!\n");
    return 0; 
}