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
    PAWN = 2, // start at two because index 0 & 1 represent color, the enum above
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    EN_PASSANTE
} PIECE;

typedef enum
{
    WQS = 1,
    WKS = 2,
    BQS = 4,
    BKS = 8,
} CASTLE_RIGHT;

#define WHITE_CASTLE ((CASTLE_RIGHT)(WQS | WKS))
#define BLACK_CASTLE ((CASTLE_RIGHT)(BQS | BKS))

//-------------GAME STATE-----------------

typedef struct game_state
{
    bitboard bitboards[9]; // two for color, 6 for pieces, 1 for enpassante sq
    char castle_rights;
    unsigned int reversible_move_counter;
    unsigned int full_move_counter;
    COLOR side_to_move;
} game_state;

game_state gs_from_FEN(const char *FEN);

char *FEN_from_gs(const game_state *gs);

PIECE piece_at_sq(const game_state *gs, int sq);

bool sq_attacked(const game_state *gs, int sq, COLOR attacking_color);

void print_gamestate(const game_state *gs);

void dbg_print_gamestate(const game_state *gs);

#endif
