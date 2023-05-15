#include <stdio.h>
#include <stdlib.h>

#include "game_state.h"
#include "movegen.h"
#include "bitboard.h"

/*
void generate_knight_moves()
{
    for (int sq = 0; sq < 64; sq++)
    {
        bitboard bb_sq = BB_SQ(sq);
        bitboard b = bb_sq;
        
        b =  GEN_SHIFT(bb_sq,NORTH + NORTHWEST) & (~BB_FILE_H);
        b |= GEN_SHIFT(bb_sq,NORTH + NORTHEAST) & (~BB_FILE_A);
        b |= GEN_SHIFT(bb_sq,SOUTH + SOUTHEAST) & (~BB_FILE_A);
        b |= GEN_SHIFT(bb_sq,SOUTH + SOUTHWEST) & (~BB_FILE_H);

        b |= GEN_SHIFT(bb_sq,WEST + SOUTHWEST) & ~(BB_FILE_H | BB_FILE_G);
        b |= GEN_SHIFT(bb_sq,WEST + NORTHWEST) & ~(BB_FILE_H | BB_FILE_G);

        b |= GEN_SHIFT(bb_sq,EAST + SOUTHEAST) & ~(BB_FILE_A | BB_FILE_B);
        b |= GEN_SHIFT(bb_sq,EAST + NORTHEAST) & ~(BB_FILE_A | BB_FILE_B);

        printf("0x%016lXULL,\n",b);
    }
}
*/

int main()
{
    game_state gs = gs_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/5N2/P1PP1PPP/RNBQKB1R w KQkq - 1 2");
    //game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/pppppppp/P1PP1PPP/RNBQKB1R w KQkq - 1 2");

    print_gamestate(&gs);
    
    printf("Exiting!\n");
    return 0; 
}