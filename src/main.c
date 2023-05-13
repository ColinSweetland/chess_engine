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
    //game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/5N2/P1PP1PPP/RNBQKB1R w KQkq - 1 2");
    //game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/pppppppp/P1PP1PPP/RNBQKB1R w KQkq - 1 2");

    //print_gamestate(&gs);
    
    print_bb(bb_king_moves(0, BB_ZERO));
    print_bb(bb_king_moves(63, BB_ZERO));
    print_bb(bb_king_moves(5, BB_ZERO));
    print_bb(bb_king_moves(56, BB_ZERO));
    print_bb(bb_king_moves(7, BB_ZERO));
    print_bb(bb_king_moves(8, BB_ZERO));
    print_bb(bb_king_moves(25, BB_ZERO));
  
    printf("Exiting!\n");
    return 0; 
}