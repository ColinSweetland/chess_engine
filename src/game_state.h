#ifndef GAMESTATE_INCLUDED
#define GAMESTATE_INCLUDED

#include "bitboard.h"

//--------------ENUMS-------------------

typedef enum 
{
    WHITE,
    BLACK
} COLOR;

typedef enum 
{
    NONE_PIECE = -1,
    PAWN = 2, //start at two because index 0 & 1 represent color, the enum above
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    EN_PASSANTE
} PIECE;

typedef enum
{
    WHITE_QUEENSIDE = 1,
    WHITE_KINGSIDE = 2,
    BLACK_QUEENSIDE = 4,
    BLACK_KINGSIDE = 8,
} CASTLE_RIGHTS;

//-------------GAME STATE-----------------

typedef struct game_state
{
    bitboard bitboards[9];  // two for color, 6 for pieces, 1 for enpassante sq
    char castle_rights;
    int reversible_move_counter;
    int full_move_counter;
    COLOR side_to_move;

} game_state;

PIECE piece_at_sq(game_state *gs, int sq);

game_state gs_from_FEN(char *FEN);

char* FEN_from_gs(game_state *gs);

void dbg_print_gamestate(game_state *gs);

void print_gamestate(game_state *gs);

#endif