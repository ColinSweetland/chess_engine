#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "game_state.h"
#include "bitboard.h"

// ---------------- MOVES ---------------------------

void print_move(chess_move move)
{
    printf("\n%c %d -> %c %d\n",
    'a' + move.from_square % 8,
    (move.from_square / 8) + 1,
    'a' + move.to_square % 8,
    (move.to_square / 8) + 1
    );
}

// --------------- GAME STATE ---------------------

static game_state get_gamestate() 
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

void print_gamestate(game_state *gs)
{
    char *FEN = FEN_from_gs(gs);
    int FEN_index = 0;

    // print out 8 rows of pieces
    for(int rank = 8; rank >= 1; rank--)
    {

        printf("\n%d\t", rank); // print row # label

        // each row is until / or space (last row)
        while(FEN[FEN_index] != '/' && FEN[FEN_index] != ' ')
        {

            // if it's a space #, put that many .
            if(isdigit(FEN[FEN_index]))
            {
                for(int spaces = FEN[FEN_index] - '0'; spaces > 0; spaces--)
                {
                    printf(" . ");
                }
            } 
            else // put the character piece
            {
                printf(" %c ", FEN[FEN_index]);
            }

            FEN_index++;
        } 

        // print other game info
        switch(rank)
        {
            case(7) :
                printf("\tFull Moves: %d Rev Moves: %d",gs->full_move_counter,gs->reversible_move_counter);
                break;
            
            case(5) :
                printf("\tCastling: ");
                if (gs->castle_rights &  WHITE_KINGSIDE)
                    putchar('K');
                if (gs->castle_rights &  WHITE_QUEENSIDE)
                    putchar('Q');  
                if (gs->castle_rights &  BLACK_KINGSIDE)
                    putchar('k');
                if (gs->castle_rights &  BLACK_QUEENSIDE)
                    putchar('q');
                if (gs->castle_rights == 0)
                    putchar('-');
                break;
            
            case(3) : 
                if (gs->en_passante_target != -1) 
                {
                    printf("\tEn Passante Available: %c%c", 
                    FILE_CHAR_FROM_SQ(gs->en_passante_target),
                    RANK_CHAR_FROM_SQ(gs->en_passante_target)); 
                } else 
                {
                    printf("\tNo En Passante");
                }
                break;
            
            case(1) :
                printf("\t%s to move", gs->side_to_move ? "Black" : "White");
                break;
        }


        FEN_index++;
    }

    // print file label
    printf("\n\n\t a  b  c  d  e  f  g  h\n");
}

// prints the gamestate in an extremely verbose way, for debugging only
void dbg_print_gamestate(game_state *gs) 
{
    printf("\n*** PAWNS ***\n");
    print_bb(gs->bitboards[PAWN]);

    printf("\n*** ROOKS ***\n");
    print_bb(gs->bitboards[ROOK]);

    printf("\n*** KNIGHTS ***\n");
    print_bb(gs->bitboards[KNIGHT]);

    printf("\n*** BISHOPS ***\n");
    print_bb(gs->bitboards[BISHOP]);

    printf("\n*** QUEENS ***\n");
    print_bb(gs->bitboards[QUEEN]);

    printf("\n*** KINGS ***\n");
    print_bb(gs->bitboards[KING]);

    printf("\n*** WHITE PIECES ***\n");
    print_bb(gs->bitboards[WHITE]);

    printf("\n*** BLACK PIECES ***\n");
    print_bb(gs->bitboards[BLACK]);

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


// Forsyth Edwards Notation is a common string based representation of gamestate for chess
// https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

game_state gs_from_FEN(char* FEN) 
{
    game_state gs = get_gamestate();

    // Fen strings start at top left position
    int bb_file = 1;
    int bb_rank = 8;
    char *FEN_CHAR = FEN;

    int sq = 56;

    // board position section
    while(sq != 7)
    {
        sq = RANKFILE_TO_SQ(bb_rank, bb_file);

        // if we find a number: move num - 1 squares
        if (*FEN_CHAR >= '1' && *FEN_CHAR <= '8') 
        {
            bb_file += *FEN_CHAR - '0';
        } 
        // if we found a / move down a rank and to the first file
        else if (*FEN_CHAR == '/') 
        {
            bb_file = 1;
            bb_rank -= 1;
        }   
        // if we found a character we found a piece
        else if (isalpha(*FEN_CHAR))
        {
            // black is lowercase, white uppercase
            BB_SET(gs.bitboards[!(islower(*FEN_CHAR) == 0)], sq);

            switch(tolower(*FEN_CHAR))
            {
                case('p') :
                    BB_SET(gs.bitboards[PAWN], sq);
                    break;
                case('n') :
                    BB_SET(gs.bitboards[KNIGHT], sq);
                    break;
                case('b') :
                    BB_SET(gs.bitboards[BISHOP], sq);
                    break;
                case('r') :
                    BB_SET(gs.bitboards[ROOK], sq);
                    break;
                case('q') :
                    BB_SET(gs.bitboards[QUEEN], sq);
                    break;
                case('k') :
                    BB_SET(gs.bitboards[KING], sq);
                    break;
                
                default :
                    fprintf(stderr, "*** ERROR PARSING FEN ***\n");
                    fprintf(stderr, "FEN: %s\n", FEN);
                    fprintf(stderr, "Found unexpected character: %c", *FEN_CHAR);
                    exit(1);
                    break;
            }

            bb_file++;
        }

        FEN_CHAR += 1;
    }
    
    FEN_CHAR++; // skip space

    // ---- which side moves ----
    gs.side_to_move = *FEN_CHAR == 'b';
    


    FEN_CHAR += 2; // first char of next section
    

    // ---- castling rights ----
    // '-' indicates nobody can castle
    if (*FEN_CHAR != '-')
    {
        // space indicates end of section
        while(*FEN_CHAR != ' ' )
        {
            switch(*FEN_CHAR)
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

            FEN_CHAR++;
        }
    }
 
    FEN_CHAR++; // skip space

    // ---- EN PASSANTE TARGET SQUARE ----
    // '-' indicates no enpassante available
    if (*FEN_CHAR != '-') 
    {
        // FILE LETTER
        gs.en_passante_target = *FEN_CHAR - 'a';
        FEN_CHAR++;
        gs.en_passante_target += (*FEN_CHAR - '1') * 8;
    }

    FEN_CHAR += 2;



    // ---- REVERSIBLE or HALF MOVE COUNTER ----
    gs.reversible_move_counter = (int) strtol(FEN_CHAR, &FEN_CHAR, 10);

    // ---- FULL MOVE COUNTER ----
    gs.full_move_counter = (int) strtol(FEN_CHAR, &FEN_CHAR, 10);

    return gs;
}

char* FEN_from_gs(game_state *gs) 
{
    // theoretical max length FEN is like 87 (according to stack overflow) + rounding to extra safe
    char* FEN = calloc(90, sizeof(char));

    // "Lookup tables"
    // index with : Color + ((Piece - 2) * 2)
    char* piece_letters = "PpNnBbRrQqKk";
    char* side_to_move = "wb";

    int FEN_index = 0; // index in FEN

    int piece_color = 0;

    bool is_empty_space = 0;
    int empty_space_count = 0;
     
    // ---- PIECES ----
    // through each bit board index
    for (int bb_index = 56; bb_index >= 0; bb_index++) 
    {
        is_empty_space = 1;

        // go through all piece bitboards looking for set bit at bb index
        for(int piece = PAWN; piece <= KING; piece++)
        {
            //if there is a piece at the index
            if (BB_IS_SET_AT(gs->bitboards[piece], bb_index)) 
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
        if (FILE_FROM_SQ(bb_index) == 8) 
        {
            // if we have empty space at the end of the line..
            // we must write it, since it can't carry to the next line. 
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
        FEN[FEN_index++] = FILE_CHAR_FROM_SQ(gs->en_passante_target);
        // RANK
        FEN[FEN_index++] = RANK_CHAR_FROM_SQ(gs->en_passante_target);
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