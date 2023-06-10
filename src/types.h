#ifndef TYPES_INCL
#define TYPES_INCL

#include <cinttypes>

/* This file will define the basic types used in the engine */

using bitboard = std::uint64_t;

// 0-63 square on bitboard, from a1 to h8. -1 used for no square
using square = int;

enum COLOR
{
    WHITE,
    BLACK
};

// direction on the board, from white's point of view
enum DIR
{
    NORTH = 8,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTHWEST = NORTH + WEST,
    NORTHEAST = NORTH + EAST,
    SOUTHEAST = SOUTH + EAST,
    SOUTHWEST = SOUTH + WEST,
};

#define PAWN_PUSH_DIR(color) ((color) ? SOUTH : NORTH)

enum PIECE
{
    NONE_PIECE = -1,
    PAWN = 2, // start at two because index 0 & 1 represent color, the enum above
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    EN_PASSANTE
};

enum CASTLE_RIGHT
{
    WQS = 1,
    WKS = 2,
    BQS = 4,
    BKS = 8,

    WCR = WQS | WKS,
    BCR = BQS | BKS
};

struct game_state
{
    bitboard bitboards[9]; // two for color, 6 for pieces, 1 for enpassante sq
    char castle_rights;

    unsigned int reversible_move_counter;
    unsigned int full_move_counter;

    COLOR side_to_move;
};

struct chess_move
{
    square origin;
    square dest;

    PIECE movedp;
    PIECE captp;
    PIECE promo; // we indicate castling by setting promo to king/queen to indicate king/queenside
};

#endif // TYPES_INCL