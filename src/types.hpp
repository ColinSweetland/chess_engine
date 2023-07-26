#ifndef TYPES_INCL
#define TYPES_INCL

#include "util.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/* This file will define the basic types used in the engine */

// shorter alias for stdlib string
using str = std::string;

using bitboard = std::uint64_t;

// 0-63 square on bitboard, from a1 to h8. -1 sometimes used for no square
using square = std::int8_t;

enum COLOR
{
    NO_COLOR = -1,
    WHITE    = 0,
    BLACK    = 1
};

// color flip with just ! and no static_cast
constexpr COLOR operator!(COLOR c) { return static_cast<COLOR>(!util::to_underlying(c)); }

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

// add directions with + operator and no static cast
constexpr DIR operator+(DIR l, DIR r) { return static_cast<DIR>(util::to_underlying(l) + util::to_underlying(r)); }

// use unary - to flip direction (e.g. north to south)
constexpr DIR operator-(DIR d) { return static_cast<DIR>(-util::to_underlying(d)); };

constexpr DIR PAWN_PUSH_DIR(COLOR c) { return c ? SOUTH : NORTH; }

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

// unit for evaluation. 100 = value of a pawn.
using centipawn = int32_t;

#endif // TYPES_INCL