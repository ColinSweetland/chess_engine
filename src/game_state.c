#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "game_state.h"
#include "bitboard.h"

// prints the gamestate in an extremely verbose way, for debugging only
void dbg_print_gamestate(game_state *gs) 
{
    printf("\n*** PAWNS ***\n");
    print_bitboard(gs->bitboards[PAWN]);

    printf("\n*** ROOKS ***\n");
    print_bitboard(gs->bitboards[ROOK]);

    printf("\n*** KNIGHTS ***\n");
    print_bitboard(gs->bitboards[KNIGHT]);

    printf("\n*** BISHOPS ***\n");
    print_bitboard(gs->bitboards[BISHOP]);

    printf("\n*** QUEENS ***\n");
    print_bitboard(gs->bitboards[QUEEN]);

    printf("\n*** KINGS ***\n");
    print_bitboard(gs->bitboards[KING]);

    printf("\n*** WHITE PIECES ***\n");
    print_bitboard(gs->bitboards[WHITE]);

    printf("\n*** BLACK PIECES ***\n");
    print_bitboard(gs->bitboards[BLACK]);

    printf("\n*** OTHER DATA ***\n");
    printf("CASTLE RIGHTS\n");
    printf("WHITE KINGSIDE: %d\n",(gs->castle_rights & WHITE_KINGSIDE) > 0);
    printf("WHITE QUEENSIDE: %d\n",(gs->castle_rights & WHITE_QUEENSIDE) > 0);
    printf("BLACK KINGSIDE: %d\n",(gs->castle_rights & BLACK_KINGSIDE) > 0);
    printf("BLACK QUEENSIDE: %d\n",(gs->castle_rights & BLACK_QUEENSIDE) > 0);

    printf("EN PASSANTE TARGET: %d\n",gs->en_passante_target);
    printf("TO MOVE: %d\n", gs->side_to_move);
    printf("FULL MOVE COUNTER: %d\n", gs->full_move_counter);
    printf("REVERSIBLE MOVE COUNTER: %d\n", gs->reversible_move_counter);
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
    gs.en_passante_target = -1;

    gs.full_move_counter = 1;

    gs.side_to_move = WHITE;

    gs.reversible_move_counter = 0;

    return gs;
}

// Forsyth Edwards Notation is a common string based representation of gamestate for chess
// https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

game_state gs_from_FEN(char* FEN) 
{
    game_state gs = get_initialized_gamestate();

    char* FEN_cptr = FEN;

    // Fen strings start at top left position,
    // AKA index 56
    int bb_index = 8 * 7;
        
    // board position section
    while(*FEN_cptr != ' ') 
    {
        switch(*FEN_cptr) 
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
        FEN_cptr += 1;
    }
    
    FEN_cptr++; // skip space

    // ---- which side moves ----
    gs.side_to_move = *FEN_cptr == 'b';
    
    FEN_cptr += 2; // first char of next section

    // ---- castling rights ----
    // '-' indicates nobody can castle
    if (*FEN_cptr != '-')
    {
        // space indicates end of section
        while(*FEN_cptr != ' ')
        {
            switch(*FEN_cptr)
            {
                case('K') :
                    gs.castle_rights |= WHITE_KINGSIDE;
                    break;
                case('Q') :
                    gs.castle_rights |= WHITE_QUEENSIDE;
                    break;
                case('k') :
                    gs.castle_rights |= BLACK_KINGSIDE;
                    break;
                case('q') :
                    gs.castle_rights |= BLACK_QUEENSIDE;
                    break;
            }

            FEN_cptr++;
        }
    }
 
    FEN_cptr++; // skip space

    // ---- EN PASSANTE TARGET SQUARE ----
    // '-' indicates no enpassante available
    if (*FEN_cptr != '-') 
    {
        // FILE LETTER
        gs.en_passante_target = *FEN_cptr - 'a';

        // RANK NUM
        FEN_cptr++;
        gs.en_passante_target += (*FEN_cptr - '1') * 8;
    }

    FEN_cptr += 2;

    // ---- REVERSIBLE or HALF MOVE COUNTER ----
    gs.reversible_move_counter = (int) strtol(FEN_cptr, &FEN_cptr, 10);

    // ---- FULL MOVE COUNTER ----
    gs.full_move_counter = (int) strtol(FEN_cptr, &FEN_cptr, 10);

    return gs;
}

char* FEN_from_gs(game_state *gs) 
{
    // theoretical max length FEN is like 87 (according to stack overflow) + rounding to extra safe
    char* FEN = calloc(90, sizeof(char));

    // "Lookup tables"
    // index with : Color + ((Piece - 2) * 2)
    char* piece_letters = "PpRrNnBbQqKk";
    char* side_to_move = "wb";

    int FEN_index = 0; // index in FEN

    int piece_color = 0;
    int file = 0;

    int is_empty_space = 0;
    int empty_space_count = 0;
     
    // ---- PIECES ----
    // through each bit board index
    for (int bb_index = 56; bb_index >= 0; bb_index++) 
    {

        file = bb_index % 8;
        is_empty_space = 1;

        // go through all piece bitboards looking for set bit at bb index
        for(int piece = PAWN; piece <= KING; piece++)
        {
            //if there is a piece at the index
            if (is_set_at_index(gs->bitboards[piece], bb_index)) 
            {
                
                is_empty_space = 0;

                // write the number of empty spaces if we have some
                if (empty_space_count > 0) {
                    
                    FEN[FEN_index++] = empty_space_count + '0';

                    empty_space_count = 0;
                }

                // if the bit intersects with black, the piece is black, otherwise white
                piece_color = (((bitboard) pow(2, bb_index)) & gs->bitboards[BLACK]) > 1;

                // write the appropriate piece to the FEN string
                FEN[FEN_index++] = piece_letters[piece_color + ((piece - 2) * 2)];
            }
        }

        if (is_empty_space) {
            empty_space_count++;
        } 

        // last column
        if (file == 7) 
        {
            if (empty_space_count > 0) 
            {
            
                // write the last empty space for the line
                FEN[FEN_index++] = empty_space_count + '0';
                
                empty_space_count = 0;
            }

            // add / which means end of row, except on the last row (row 1)
            if (bb_index != 7)
            {
                FEN[FEN_index++] = '/';
            }

            // shift bb index to next row, first entry
            bb_index -= 16;
        }
    }

    // --- SIDE TO MOVE ---
    FEN[FEN_index++] = ' ';
    FEN[FEN_index++] = side_to_move[gs->side_to_move];
    FEN[FEN_index++] = ' ';

    // --- CASTLE RIGHTS ---
    if(gs->castle_rights == 0) 
    {
        FEN[FEN_index++] = '-';
    } 
    else 
    {
        if(gs->castle_rights & WHITE_KINGSIDE)
        {
            FEN[FEN_index++] = 'K';
        }
        if(gs->castle_rights & WHITE_QUEENSIDE)
        {
            FEN[FEN_index++] = 'Q';
        }
        if(gs->castle_rights & BLACK_KINGSIDE)
        {
            FEN[FEN_index++] = 'k';
        }
        if(gs->castle_rights & BLACK_QUEENSIDE)
        {
            FEN[FEN_index++] = 'q';
        }
    }

    // --- EN PASSANTE ---

    FEN[FEN_index++] = ' ';
    
    // no enpassante available
    if (gs->en_passante_target < 0) 
    {
        FEN[FEN_index++] = '-';
    }
    else 
    {
        // FILE
        FEN[FEN_index++] = (gs->en_passante_target % 8) + 'a';
        // RANK
        FEN[FEN_index++] = gs->en_passante_target / 8 + '1';
    }

    FEN[FEN_index++] = ' ';

    // --- HALF/REVERSIBLE MOVES --- 
    // returns chars written
    FEN_index += sprintf(FEN + FEN_index, "%d", gs->reversible_move_counter);
    FEN[FEN_index++] = ' ';

    // --- FULL MOVE COUNTER ---
    FEN_index += sprintf(FEN + FEN_index, "%d", gs->full_move_counter);
    
    // truncate string
    // This is like a pseudo-leak (leaves a small unused portion of the string)
    FEN[FEN_index] = '\0';

    return FEN;
}