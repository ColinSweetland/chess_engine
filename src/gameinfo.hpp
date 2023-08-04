#ifndef GAMEINFO_INCL
#define GAMEINFO_INCL

#include "./types/bitboard.hpp"
#include "./types/pieces.hpp"

// this color can promote on what rank
constexpr int promo_rank_num(COLOR c) { return c ? 1 : 8; }

// this color pushes their pawns which way
constexpr DIR push_dir(COLOR c) { return c ? SOUTH : NORTH; }

#endif // GAMEINFO_INCL