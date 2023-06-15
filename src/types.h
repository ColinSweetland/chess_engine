#ifndef TYPES_INCL
#define TYPES_INCL

#include <cinttypes>
#include <string>
#include <unordered_map>

/* This file will define the basic types used in the engine */

// alias for stdlib string
using str = std::string;

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
    EAST  = 1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTHWEST = NORTH + WEST,
    NORTHEAST = NORTH + EAST,
    SOUTHEAST = SOUTH + EAST,
    SOUTHWEST = SOUTH + WEST,
};

inline DIR PAWN_PUSH_DIR(COLOR c) { return c ? SOUTH : NORTH; }

enum PIECE
{
    NO_PIECE = -1,
    PAWN     = 2, // start at two because index 0 & 1 represent color
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    EN_PASSANTE
};

const std::unordered_map<char, PIECE> char_to_piece{{'p', PAWN}, {'n', KNIGHT}, {'b', BISHOP},
                                                    {'r', ROOK}, {'q', QUEEN},  {'k', KING}};

const std::unordered_map<int, char> piece_to_char{{WHITE, 'w'},       {BLACK, 'b'},   {PAWN, 'p'},  {KNIGHT, 'n'},
                                                  {BISHOP, 'b'},      {ROOK, 'r'},    {QUEEN, 'q'}, {KING, 'k'},
                                                  {EN_PASSANTE, 'e'}, {NO_PIECE, '.'}};

enum CASTLE_RIGHT
{
    NO_RIGHTS = 0,
    WQS       = 1,
    WKS       = 2,
    BQS       = 4,
    BKS       = 8,

    WCR = WQS | WKS,
    BCR = BQS | BKS
};

struct chess_move
{
    square from_sq;
    square to_sq;

    PIECE movedp;
    PIECE captp;
    PIECE promo; // we indicate castling by setting promo to king/queen to indicate king/queenside
};

#endif // TYPES_INCL