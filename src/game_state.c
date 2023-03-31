#include <stdlib.h>
#include <stdio.h>

#include "game_state.h"
#include "bitboard.h"

// prints the gamestate (currently only piece info) in a 
// extremely verbose way, for debugging only
void dbg_print_gamestate(game_state gs) 
{
    printf("\n*** BLACK PAWNS ***\n");
    print_bitboard(gs.black_bitboards[PAWN]);

    printf("\n*** BLACK ROOK ***\n");
    print_bitboard(gs.black_bitboards[ROOK]);

    printf("\n*** BLACK KNIGHT ***\n");
    print_bitboard(gs.black_bitboards[KNIGHT]);

    printf("\n*** BLACK BISHOP ***\n");
    print_bitboard(gs.black_bitboards[BISHOP]);

    printf("\n*** BLACK QUEEN ***\n");
    print_bitboard(gs.black_bitboards[QUEEN]);

    printf("\n*** BLACK KING ***\n");
    print_bitboard(gs.black_bitboards[KING]);

    printf("\n*** WHITE PAWNS ***\n");
    print_bitboard(gs.white_bitboards[PAWN]);

    printf("\n*** WHITE ROOK ***\n");
    print_bitboard(gs.white_bitboards[ROOK]);

    printf("\n*** WHITE KNIGHT ***\n");
    print_bitboard(gs.white_bitboards[KNIGHT]);

    printf("\n*** WHITE BISHOP ***\n");
    print_bitboard(gs.white_bitboards[BISHOP]);

    printf("\n*** WHITE QUEEN ***\n");
    print_bitboard(gs.white_bitboards[QUEEN]);

    printf("\n*** WHITE KING ***\n");
    print_bitboard(gs.white_bitboards[KING]);


}

game_state get_initialized_gamestate() 
{
    game_state gs = *(game_state *) malloc(sizeof(game_state));
    


    // set all black bitboards to 0 (otherwise they will be random) 
    for (int i = 0; i < 7; i++)
    {
       gs.black_bitboards[i] &= 0;
    }

    // ditto for white
    for (int i = 0; i < 7; i++)
    {
        gs.white_bitboards[i] &= 0;
    }

    // we won't set full castle rights 
    // because most often we use generate from FEN after this 
    // and we would just have to set it to 0 there
    gs.castle_rights = 0;

    // not implemented
    gs.EN_PASSANTE = -1;

    gs.full_move_counter = 1;

    gs.side_to_move = WHITE;

    gs.reversible_move_counter = 0;

    return gs;
}


// STARTING FEN
// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

// Forsyth Edwards Notation is a common string based representation of gamestate for chess
// https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

game_state gs_from_FEN(char* FEN) 
{
    game_state gs = get_initialized_gamestate();

    int index = 0;

    // Fen strings start at top left position,
    // AKA index 56
    int bb_index = 8 * 7;
        
    // board position section
    while(FEN[index] != ' ') 
    {
        switch(FEN[index]) {
            
            case('p') : // black pawn
                set_bboard_index(&gs.black_bitboards[PAWN], bb_index);
                break;

            case('r') : // black rook
                set_bboard_index(&gs.black_bitboards[ROOK], bb_index);
                break;

            case('n') : // black knight
                set_bboard_index(&gs.black_bitboards[KNIGHT], bb_index);
                break;

            case('b') : // black bishop
                set_bboard_index(&gs.black_bitboards[BISHOP], bb_index);
                break;

            case('q') : // black queen
                set_bboard_index(&gs.black_bitboards[QUEEN], bb_index);
                break;
            
            case('k') : // black king
                set_bboard_index(&gs.black_bitboards[KING], bb_index);
                break;

            case('P') : // white pawn
                set_bboard_index(&gs.white_bitboards[PAWN], bb_index);
                break;

            case('R') : // white rook
                set_bboard_index(&gs.white_bitboards[ROOK], bb_index);
                break;

            case('N') : // white knight
                set_bboard_index(&gs.white_bitboards[KNIGHT], bb_index);
                break;

            case('B') : // white bishop
                set_bboard_index(&gs.white_bitboards[BISHOP], bb_index);
                break;

            case('Q') : // white queen
                set_bboard_index(&gs.white_bitboards[QUEEN], bb_index);
                break;

            case('K') : // white king
                set_bboard_index(&gs.white_bitboards[KING], bb_index);
                break;

            case('2') : // all numbers add themselvs - 1 (because we add 1 at end of loop)
                bb_index += 1;
                break;
            case('3') :
                bb_index += 2;
                break;
            case('4') :
                bb_index += 3;
                break;
            case('5') :
                bb_index += 4;
                break;
            case('6') :
                bb_index += 5;
                break;
            case('7') :
                bb_index += 6;
                break;
            case('8') :
                bb_index += 7; 
                break;
            case('/') : // next rank
        
                bb_index -= 17; // -8 (down direction) -8 (8 spaces left) - 1 (because we add one at end of loop)
                break;
        }

        bb_index += 1;
        index += 1;
    }


    return gs;
}