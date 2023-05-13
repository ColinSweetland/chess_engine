#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "movegen.h"
#include "bitboard.h"
#include "game_state.h"

// when we shift east/west, wrapping can happen. To avoid this, we have to mask a row
static const bitboard e_mask = ~BB_FILE_A;
static const bitboard w_mask = ~BB_FILE_H;

static int is_within_inc(int num, int low, int high)
{
    return num >= low && num <= high;
}

// parse a move in the UCI format ("Long Algebraic notation") e.g. a2a4 or b7b8Q 
chess_move parse_move(char *movestring)
{
    char *promotions = "RNBQ"; // all promotable pieces
    chess_move move = {0, 0, NONE_PIECE, NONE_PIECE, NONE_PIECE, 0};
    int idx = 0;

    // FROM SQUARE
    if (!is_within_inc(movestring[idx], 'a', 'h') || !is_within_inc(movestring[idx + 1], '1', '8'))
    {
        goto PARSE_ERR;
    } 

    move.from_square = (movestring[idx] - 'a') + (movestring[idx + 1] - '1') * 8;

    idx += 2;

    // TO SQUARE
    if (!is_within_inc(movestring[idx], 'a', 'h') || !is_within_inc(movestring[idx + 1], '1', '8'))
    {
        goto PARSE_ERR;
    }

    move.to_square = (movestring[idx] - 'a') + (movestring[idx + 1] - '1') * 8;

    idx += 2;

    // PROMOTION
    for (int i = 0; i < 4; i++)
    {
        if (promotions[i] == movestring[idx])
        {
            move.promotion = ROOK + i;
        }
    }

    // wasn't assigned and it's not a space or NULL string = ERR
    if (move.promotion == -1 && !(isspace(movestring[idx]) || movestring[idx] == '\0'))
    {
        goto PARSE_ERR;
    }
    
    return move;   

    PARSE_ERR :
    fprintf(stderr,"ERR: CAN'T PARSE MOVE \"%s\" \n",movestring);
    fprintf(stderr,"EXITING\n");
    exit(1);
}


// ------------------KINGS-----------------------

// The thrill is gone

// this may be worth creating a lookup table for and not too hard
bitboard bb_king_moves(game_state *gs, COLOR side_to_move)
{
    bitboard king = gs->bitboards[KING] & gs->bitboards[side_to_move];
    bitboard moves = BB_ZERO;

    moves |= GEN_SHIFT(king, NORTHWEST) & w_mask;
    moves |= GEN_SHIFT(king, NORTH    );
    moves |= GEN_SHIFT(king, NORTHEAST) & e_mask;
    moves |= GEN_SHIFT(king, EAST     ) & e_mask;
    moves |= GEN_SHIFT(king, SOUTHEAST) & e_mask;
    moves |= GEN_SHIFT(king, SOUTH    );
    moves |= GEN_SHIFT(king, SOUTHWEST) & w_mask;
    moves |= GEN_SHIFT(king, WEST     ) & w_mask;
    
    return moves & ~(gs->bitboards[side_to_move]);
}


// ------------------PAWNS-----------------------
// 1. attacks
bitboard bb_pawn_attacks_w(game_state* gs, COLOR side_to_move)
{
    bitboard pawns = gs->bitboards[side_to_move];
    
    direction attack_dir = side_to_move == BLACK ? SOUTHWEST : NORTHWEST;

    return GEN_SHIFT(pawns, attack_dir) & gs->bitboards[!side_to_move] & w_mask;
}

bitboard bb_pawn_attacks_e(game_state* gs, COLOR side_to_move)
{
    bitboard pawns = gs->bitboards[side_to_move];
    
    direction attack_dir = side_to_move == BLACK ? SOUTHEAST : NORTHEAST;

    return GEN_SHIFT(pawns, attack_dir) & gs->bitboards[!side_to_move] & e_mask;
}


// 2. Moves
bitboard bb_pawn_single_moves(game_state *gs, COLOR side_to_move)
{
    direction move_dir = side_to_move == BLACK ? SOUTH : NORTH;

    bitboard occupancy = gs->bitboards[WHITE] | gs->bitboards[BLACK];
    bitboard pawns = gs->bitboards[PAWN] & gs->bitboards[side_to_move];

    return GEN_SHIFT(pawns, move_dir) & ~occupancy;
}

bitboard bb_pawn_double_moves(game_state *gs, bitboard single_moves, COLOR side_to_move)
{
    bitboard occupancy = gs->bitboards[WHITE] | gs->bitboards[BLACK];

    direction move_dir; 
    bitboard double_move_rank;
    
    // BLACK
    if (side_to_move == BLACK)
    {
        move_dir = SOUTH;
        double_move_rank = BB_RANK_5;
    }
    // WHITE
    else
    {
        move_dir = NORTH;
        double_move_rank = BB_RANK_4;
    }

    return GEN_SHIFT(single_moves, move_dir) & ~occupancy & double_move_rank;
}
