#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
// need for PEXT/PDEP
#include <x86intrin.h> 

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

// ------------------ROOKS-----------------------

// lookup moves [idxlookup[sq] + PEXT(blockers)]
// we can make this more memory eff later (many elements are copies of eachother)
// size: ~800kb (too much)
static bitboard ROOK_MOVE_LOOKUP[102337] = {0};

// squares set might contain pieces that block rook moves
static const bitboard ROOK_BLOCKER_MASK[64] = 
{
    0x000101010101017e, 0x000202020202027c, 0x000404040404047a,
    0x0008080808080876, 0x001010101010106e, 0x002020202020205e,
    0x004040404040403e, 0x008080808080807e, 0x0001010101017e00,
    0x0002020202027c00, 0x0004040404047a00, 0x0008080808087600,
    0x0010101010106e00, 0x0020202020205e00, 0x0040404040403e00,
    0x0080808080807e00, 0x00010101017e0100, 0x00020202027c0200,
    0x00040404047a0400, 0x0008080808760800, 0x00101010106e1000,
    0x00202020205e2000, 0x00404040403e4000, 0x00808080807e8000,
    0x000101017e010100, 0x000202027c020200, 0x000404047a040400,
    0x0008080876080800, 0x001010106e101000, 0x002020205e202000,
    0x004040403e404000, 0x008080807e808000, 0x0001017e01010100,
    0x0002027c02020200, 0x0004047a04040400, 0x0008087608080800,
    0x0010106e10101000, 0x0020205e20202000, 0x0040403e40404000,
    0x0080807e80808000, 0x00017e0101010100, 0x00027c0202020200,
    0x00047a0404040400, 0x0008760808080800, 0x00106e1010101000,
    0x00205e2020202000, 0x00403e4040404000, 0x00807e8080808000,
    0x007e010101010100, 0x007c020202020200, 0x007a040404040400,
    0x0076080808080800, 0x006e101010101000, 0x005e202020202000,
    0x003e404040404000, 0x007e808080808000, 0x7e01010101010100,
    0x7c02020202020200, 0x7a04040404040400, 0x7608080808080800,
    0x6e10101010101000, 0x5e20202020202000, 0x3e40404040404000,
    0x7e80808080808000
};

// Where you can begin to lookup the move bb for a certain square
// for example, in a1 we have 4095 (2^12 - 1) possible blocker combos,
// because there are 12 squares where blockers can be
// so for a2 we start at index 4095 (4096th element)
// max idx = 102336
static const uint32_t ROOK_MOVE_START_IDX[64] = 
{
        0,   4095,   6142,   8189,  10236,  12283,  14330,  16377,
    20472,  22519,  23542,  24565,  25588,  26611,  27634,  28657,
    30704,  32751,  33774,  34797,  35820,  36843,  37866,  38889,
    40936,  42983,  44006,  45029,  46052,  47075,  48098,  49121,
    51168,  53215,  54238,  55261,  56284,  57307,  58330,  59353,
    61400,  63447,  64470,  65493,  66516,  67539,  68562,  69585,
    71632,  73679,  74702,  75725,  76748,  77771,  78794,  79817,
    81864,  85959,  88006,  90053,  92100,  94147,  96194,  98241
};


// TODO: make this work properly for multiple rooks
bitboard bb_rook_moves(game_state *gs)
{
    
    int rook_sq = BB_LSB(gs->bitboards[ROOK] & gs->bitboards[gs->side_to_move]);
    
    // no rook on board -> no moves
    if (rook_sq == -1)
    {
        return BB_ZERO;
    }
    
    bitboard occ = gs->bitboards[BLACK] | gs->bitboards[WHITE];
    
    bitboard blocker_mask = ROOK_BLOCKER_MASK[rook_sq];
   
    bitboard blocker_key = BB_PEXT(occ, blocker_mask);

    return ROOK_MOVE_LOOKUP[ROOK_MOVE_START_IDX[rook_sq] + blocker_key] & ~gs->bitboards[gs->side_to_move];
}

bitboard dumb7fill(int origin_sq, bitboard blockers)
{
    int dirs[4] = {NORTH, SOUTH, EAST, WEST};
    bitboard dirmask[4] = {~BB_ZERO, ~BB_ZERO, ~BB_FILE_A, ~BB_FILE_H};

    bitboard moves_bb = BB_ZERO;

    // diri indicates the direction we are filling, and mask to apply (above two tables)
    for (int diri = 0; diri < 4; diri++)
    {
        direction current_dir = dirs[diri];
        bitboard current_dirmask = dirmask[diri];

        // empty squares = 1, occupied = 0
        bitboard prop = ~blockers & current_dirmask;

        // the square the rook is on
        bitboard r = BB_SQ(origin_sq);

        // we fill this with valid moves until blocker
        bitboard flood = BB_ZERO;

        // push rook along dir, until a blocker makes it dissapear
        while(r)
        {
            flood |= r;
            r = GEN_SHIFT(r, current_dir) & prop;
        }

        // shift one more time to get attacks/edges
        flood = GEN_SHIFT(flood, current_dir) & dirmask[diri];

        moves_bb |= flood;
    }

    return moves_bb;
}

void init_rook_tables()
{    
    // iterate over each square
    for (int curr_sq = 0; curr_sq < 64; curr_sq++)
    {
        
        // blockers can appear in any bit set here
        bitboard potential_blocker_mask = ROOK_BLOCKER_MASK[curr_sq];

        // we can have as many blockers as bits set
        int max_blockers = BB_POPCNT(potential_blocker_mask);
        
        // iterate through all combo of blockers in all relevant positions
        for(bitboard blockers = BB_ZERO; blockers < BB_SQ(max_blockers); blockers++)
        {
            // this particular combo
            bitboard blocker_set = BB_PDEP(blockers, potential_blocker_mask);

            bitboard moves = dumb7fill(curr_sq, blocker_set);

            int table_idx = ROOK_MOVE_START_IDX[curr_sq] + blockers;
            ROOK_MOVE_LOOKUP[table_idx] = moves;
        }
    }
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
