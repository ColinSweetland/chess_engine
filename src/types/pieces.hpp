#ifndef PIECES_INCL
#define PIECES_INCL
#include <cstdlib>

#include "../util.hpp"

enum COLOR : int
{
    NO_COLOR = -1,
    WHITE    = 0,
    BLACK    = 1
};

// color flip with just ! and no static_cast
constexpr COLOR operator!(COLOR c) { return static_cast<COLOR>(!util::to_underlying(c)); }

enum PIECE : int
{
    NO_PIECE = 0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    EN_PASSANTE
};

struct ColorPiece
{
    COLOR color;
    PIECE piece;
};

constexpr ColorPiece char_to_colorpiece(char c);

constexpr const char* piece_to_str(PIECE p);

constexpr char piece_to_char(PIECE piece);

constexpr char colorpiece_to_char(ColorPiece col_piece);

// =================
// == DEFINITIONS ==
// =================

constexpr char piece_to_char(PIECE piece)
{
    const char* lookupchar = ".pnbrqk";
    return lookupchar[piece];
}

constexpr char colorpiece_to_char(ColorPiece col_piece)
{
    char ret = piece_to_char(col_piece.piece);
    return col_piece.color ? ret : std::toupper(ret);
}

// allow me to compact these case statements, PLEASE
// clang-format off
constexpr ColorPiece char_to_colorpiece(char c)
{
    switch (c)
    {
        case 'p' : return {BLACK, PAWN};
        case 'P' : return {WHITE, PAWN};
        case 'n' : return {BLACK, KNIGHT};
        case 'N' : return {WHITE, KNIGHT};
        case 'b' : return {BLACK, BISHOP};
        case 'B' : return {WHITE, BISHOP};
        case 'r' : return {BLACK, ROOK};
        case 'R' : return {WHITE, ROOK};
        case 'q' : return {BLACK, QUEEN};
        case 'Q' : return {WHITE, QUEEN};
        case 'k' : return {BLACK, KING};
        case 'K' : return {WHITE, KING};
        default  : return {NO_COLOR, NO_PIECE};
    }
}

constexpr const char* piece_to_str(PIECE p)
{
    const char* lookupstr[] {"NO_PIECE", "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"};
    return lookupstr[p];
}
// clang-format on
#endif // PIECES_INCL
