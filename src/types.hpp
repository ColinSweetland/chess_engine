#ifndef TYPES_INCL
#define TYPES_INCL

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/* This file will define the basic types used in the engine */

// alias for stdlib string
using str = std::string;

using bitboard = std::uint64_t;

// 0-63 square on bitboard, from a1 to h8. -1 used for no square
using square = std::int8_t;

enum COLOR
{
    NO_COLOR = -1,
    WHITE    = 0,
    BLACK    = 1
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

const std::unordered_map<char, PIECE> char_to_piece = {{'p', PAWN}, {'n', KNIGHT}, {'b', BISHOP},
                                                       {'r', ROOK}, {'q', QUEEN},  {'k', KING}};

const std::unordered_map<int, char> piece_to_char = {{WHITE, 'w'},       {BLACK, 'b'},   {PAWN, 'p'},  {KNIGHT, 'n'},
                                                     {BISHOP, 'b'},      {ROOK, 'r'},    {QUEEN, 'q'}, {KING, 'k'},
                                                     {EN_PASSANTE, 'e'}, {NO_PIECE, '.'}};

const std::unordered_map<int, str> piece_to_str = {
    {WHITE, "WHITE"}, {BLACK, "BLACK"}, {PAWN, "PAWN"}, {KNIGHT, "KNIGHT"},           {BISHOP, "BISHOP"},
    {ROOK, "ROOK"},   {QUEEN, "QUEEN"}, {KING, "KING"}, {EN_PASSANTE, "EN_PASSANTE"}, {NO_PIECE, "NO_PIECE"}};

enum CASTLE_RIGHT
{
    CR_WQS = 1,
    CR_WKS = 2,
    CR_BQS = 4,
    CR_BKS = 8,

    CR_WKQS = CR_WKS | CR_WQS,
    CR_BKQS = CR_BKS | CR_BQS,

    CR_ANY = CR_WQS | CR_WKS | CR_BQS | CR_BKS
};

enum GAME_OVER
{
    NOT_GAME_OVER = 0,
    CHECKMATE     = 1,

    // below are all draws
    STALEMATE       = 2,
    FIFTY_MOVE_RULE = 3,

    /* NOT IMPLEMENTED
    REPITITION_DRAW = 4,
    INSUFFICIENT_MAT = 5
    */
};

#endif // TYPES_INCL