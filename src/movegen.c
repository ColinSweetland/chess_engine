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

static const bitboard king_lookup[64] = 
{
    0x0000000000000302ULL, 0x0000000000000705ULL, 0x0000000000000e0aULL,
    0x0000000000001c14ULL, 0x0000000000003828ULL, 0x0000000000007050ULL,
    0x000000000000e0a0ULL, 0x000000000000c040ULL, 0x0000000000030203ULL,
    0x0000000000070507ULL, 0x00000000000e0a0eULL, 0x00000000001c141cULL,
    0x0000000000382838ULL, 0x0000000000705070ULL, 0x0000000000e0a0e0ULL,
    0x0000000000c040c0ULL, 0x0000000003020300ULL, 0x0000000007050700ULL,
    0x000000000e0a0e00ULL, 0x000000001c141c00ULL, 0x0000000038283800ULL,
    0x0000000070507000ULL, 0x00000000e0a0e000ULL, 0x00000000c040c000ULL,
    0x0000000302030000ULL, 0x0000000705070000ULL, 0x0000000e0a0e0000ULL,
    0x0000001c141c0000ULL, 0x0000003828380000ULL, 0x0000007050700000ULL,
    0x000000e0a0e00000ULL, 0x000000c040c00000ULL, 0x0000030203000000ULL,
    0x0000070507000000ULL, 0x00000e0a0e000000ULL, 0x00001c141c000000ULL,
    0x0000382838000000ULL, 0x0000705070000000ULL, 0x0000e0a0e0000000ULL,
    0x0000c040c0000000ULL, 0x0003020300000000ULL, 0x0007050700000000ULL,
    0x000e0a0e00000000ULL, 0x001c141c00000000ULL, 0x0038283800000000ULL,
    0x0070507000000000ULL, 0x00e0a0e000000000ULL, 0x00c040c000000000ULL,
    0x0302030000000000ULL, 0x0705070000000000ULL, 0x0e0a0e0000000000ULL,
    0x1c141c0000000000ULL, 0x3828380000000000ULL, 0x7050700000000000ULL,
    0xe0a0e00000000000ULL, 0xc040c00000000000ULL, 0x0203000000000000ULL,
    0x0507000000000000ULL, 0x0a0e000000000000ULL, 0x141c000000000000ULL,
    0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL,
    0x40c0000000000000ULL
};

// The thrill is gone
bitboard bb_king_moves(int sq, bitboard blockers)
{
    return king_lookup[sq] & ~blockers;
}

//-------------------KNIGHTS---------------------
static const bitboard knight_lookup[64] = 
{
    0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000A1100ULL,
    0x0000000000142200ULL, 0x0000000000284400ULL, 0x0000000000508800ULL,
    0x0000000000A01000ULL, 0x0000000000402000ULL, 0x0000000002040004ULL,
    0x0000000005080008ULL, 0x000000000A110011ULL, 0x0000000014220022ULL,
    0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000A0100010ULL,
    0x0000000040200020ULL, 0x0000000204000402ULL, 0x0000000508000805ULL,
    0x0000000A1100110AULL, 0x0000001422002214ULL, 0x0000002844004428ULL,
    0x0000005088008850ULL, 0x000000A0100010A0ULL, 0x0000004020002040ULL,
    0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000A1100110A00ULL,
    0x0000142200221400ULL, 0x0000284400442800ULL, 0x0000508800885000ULL,
    0x0000A0100010A000ULL, 0x0000402000204000ULL, 0x0002040004020000ULL,
    0x0005080008050000ULL, 0x000A1100110A0000ULL, 0x0014220022140000ULL,
    0x0028440044280000ULL, 0x0050880088500000ULL, 0x00A0100010A00000ULL,
    0x0040200020400000ULL, 0x0204000402000000ULL, 0x0508000805000000ULL,
    0x0A1100110A000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL,
    0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL,
    0x0400040200000000ULL, 0x0800080500000000ULL, 0x1100110A00000000ULL,
    0x2200221400000000ULL, 0x4400442800000000ULL, 0x8800885000000000ULL,
    0x100010A000000000ULL, 0x2000204000000000ULL, 0x0004020000000000ULL,
    0x0008050000000000ULL, 0x00110A0000000000ULL, 0x0022140000000000ULL,
    0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010A00000000000ULL,
    0x0020400000000000ULL
};

bitboard bb_knight_moves(int sq, bitboard blockers)
{
    return knight_lookup[sq] & ~blockers;
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
