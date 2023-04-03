#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "game_state.h"
#include "bitboard.h"

// prints the gamestate (currently only piece info) in a 
// extremely verbose way, for debugging only
void dbg_print_gamestate(game_state gs) 
{
    printf("\n*** PAWNS ***\n");
    print_bitboard(gs.bitboards[PAWN]);

    printf("\n*** ROOKS ***\n");
    print_bitboard(gs.bitboards[ROOK]);

    printf("\n*** KNIGHTS ***\n");
    print_bitboard(gs.bitboards[KNIGHT]);

    printf("\n*** BISHOPS ***\n");
    print_bitboard(gs.bitboards[BISHOP]);

    printf("\n*** QUEENS ***\n");
    print_bitboard(gs.bitboards[QUEEN]);

    printf("\n*** KINGS ***\n");
    print_bitboard(gs.bitboards[KING]);

    printf("\n*** WHITE PIECES ***\n");
    print_bitboard(gs.bitboards[WHITE]);

    printf("\n*** BLACK PIECES ***\n");
    print_bitboard(gs.bitboards[BLACK]);
}

/*
void print_gamestate(game_state gs)
{

}
*/

game_state get_initialized_gamestate() 
{
    // calloc because bitboard must be all zeroes
    game_state gs = *(game_state *) calloc(1, sizeof(game_state));

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
        switch(FEN[index]) 
        {
            case('P') : // pawn white
                set_bboard_index(&gs.bitboards[PAWN], bb_index);
                set_bboard_index(&gs.bitboards[WHITE], bb_index);
                break;

            case('p') : // pawn black
                set_bboard_index(&gs.bitboards[PAWN], bb_index);
                set_bboard_index(&gs.bitboards[BLACK], bb_index);
                break;

            case('R') : // rook white
                set_bboard_index(&gs.bitboards[ROOK], bb_index);
                set_bboard_index(&gs.bitboards[WHITE], bb_index);
                break;

            case('r') : // rook black
                set_bboard_index(&gs.bitboards[ROOK], bb_index);
                set_bboard_index(&gs.bitboards[BLACK], bb_index);
                break;

            case('N') : // knight white
                set_bboard_index(&gs.bitboards[KNIGHT], bb_index);
                set_bboard_index(&gs.bitboards[WHITE], bb_index);
                break;
            
            case('n') : // knight black
                set_bboard_index(&gs.bitboards[KNIGHT], bb_index);
                set_bboard_index(&gs.bitboards[BLACK], bb_index);
                break;

            case('B') : // bishop white
                set_bboard_index(&gs.bitboards[BISHOP], bb_index);
                set_bboard_index(&gs.bitboards[WHITE], bb_index);
                break;

            case('b') : // bishop black
                set_bboard_index(&gs.bitboards[BISHOP], bb_index);
                set_bboard_index(&gs.bitboards[BLACK], bb_index);
                break;

            case('Q') : // queen white
                set_bboard_index(&gs.bitboards[QUEEN], bb_index);
                set_bboard_index(&gs.bitboards[WHITE], bb_index);
                break;

            case('q') : // queen black
                set_bboard_index(&gs.bitboards[QUEEN], bb_index);
                set_bboard_index(&gs.bitboards[BLACK], bb_index);
                break;

            case('K') : // king white
                set_bboard_index(&gs.bitboards[KING], bb_index);
                set_bboard_index(&gs.bitboards[WHITE], bb_index);
                break;

            case('k') : // king black
                set_bboard_index(&gs.bitboards[KING], bb_index);
                set_bboard_index(&gs.bitboards[BLACK], bb_index);
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

char* FEN_from_gs(game_state gs) 
{
    // theoretical max length FEN is like 87 + rounding to be safe
    char* FEN = calloc(90, sizeof(char));

    // index with : Color + ((Piece - 2) * 2)
    char* piece_letters = "PpRrNnBbQqKk";

    char* digits = "123456789"; // easier than sprintf & needs less buffer

    int FEN_index = 0; // index in FEN

    int piece_color = 0;
    int file = 0;

    int is_empty_space = 0;
    int empty_space_count = 0;
     
    // through each bit board index
    for (int bb_index = 56; bb_index >= 0; bb_index++) 
    {

        file = bb_index % 8;
        is_empty_space = 1;

        // go through all bitboards looking for set bit at bb index
        for(int piece = PAWN; piece <= KING; piece++)
        {
            // there is a piece here
            if (is_set_at_index(gs.bitboards[piece], bb_index)) 
            {
                
                is_empty_space = 0;

                // write the number of empty spaces if we have some
                if (empty_space_count > 0) {
                    
                    FEN[FEN_index] = digits[empty_space_count - 1];
                    FEN_index += 1;

                    empty_space_count = 0;
                }

                // if the bit intersects with black, the piece is black, otherwise white
                piece_color = (((bitboard) pow(2, bb_index)) & gs.bitboards[BLACK]) > 1;

                // write the appropriate piece to the FEN string
                FEN[FEN_index] = piece_letters[piece_color + ((piece - 2) * 2)];
                FEN_index += 1;
            }
        }

        if (is_empty_space) {
            empty_space_count += 1;
        } 

        // last column
        if (file == 7) 
        {
            if (empty_space_count > 0) 
            {
            
                // write the last empty space for the line
                FEN[FEN_index] = digits[empty_space_count - 1];
                FEN_index += 1;

                empty_space_count = 0;
            }

            // add / which means end of row, except on the last row (row 1)
            if (bb_index != 7)
            {
                FEN[FEN_index] = '/';
                FEN_index += 1;
            }

            // shift bb index to next row, first entry
            bb_index -= 16;
        }
    }

    // TODO: Create the rest of the FEN string

    return FEN;
}