#ifndef BITBOARD_INCL
#define BITBOARD_INCL

#include "types.h"

#include <cassert>
#include <x86intrin.h>
// empty: all zeroes

constexpr bitboard BB_ZERO{0ULL};

// bb at a certain square
// maybe we could use a lookup table later
constexpr bitboard BB_SQ(square sq) { return (1ULL << (sq)); }

// x -> 1 at idx
inline void BB_SET(bitboard& bb, square sq) { bb |= BB_SQ(sq); }

// x -> 0 at idx
inline void BB_UNSET(bitboard& bb, square sq) { bb &= ~BB_SQ(sq); }

// x -> ~x at idx
inline void BB_TOGGLE(bitboard& bb, square sq) { bb ^= BB_SQ(sq); }

// True if bitboard is set at idx else false
inline bool BB_IS_SET_AT(const bitboard& bb, square sq) { return bb & BB_SQ(sq); }

// "human" coordinate to bb index e.g. (3,1) -> 16
constexpr square rankfile_to_sq(int rank, int file) { return (rank - 1) * 8 + file - 1; }

// index like 56 to 1-8 file or rank num
constexpr int RANK_FROM_SQ(square sq) { return (sq / 8) + 1; }
constexpr int FILE_FROM_SQ(square sq) { return (sq % 8) + 1; }

// getting char versions of rank or file, for printing
constexpr char RANK_CHAR_FROM_SQ(square sq) { return static_cast<char>(RANK_FROM_SQ(sq) + '1' - 1); }
constexpr char FILE_CHAR_FROM_SQ(square sq) { return static_cast<char>(FILE_FROM_SQ(sq) + 'a' - 1); }

//**** GCC BUILTINS ********
// these can be replaced later if needed
constexpr square BB_LSB(const bitboard& b) { return __builtin_ffsll(b) - 1; }

inline void BB_UNSET_LSB(bitboard& b) { b &= b - 1; }

// we can use this software implementation of PEXT / PDEP later
// https://github.com/zwegner/zp7

// returns a bitboard with the lowest bits corresponding to
// the bits in bb in the positions set in mask
constexpr bitboard BB_PEXT(const bitboard& bb, const bitboard& mask) { return _pext_u64(bb, mask); }

// returns a bitboard where lowest bits of bb are moved
// into positions set in mask
constexpr bitboard BB_PDEP(const bitboard& bb, const bitboard& mask) { return _pdep_u64(bb, mask); }

// bit count
constexpr int BB_POPCNT(const bitboard& bb) { return __builtin_popcountll(bb); }

inline bitboard GEN_SHIFT(const bitboard& bb, const bitboard& dir) { return dir > 0 ? (bb << dir) : (bb >> (-dir)); }

// each board row
constexpr bitboard BB_RANK_1{0x00000000000000FFULL};
constexpr bitboard BB_RANK_2{0x000000000000FF00ULL};
constexpr bitboard BB_RANK_3{0x0000000000FF0000ULL};
constexpr bitboard BB_RANK_4{0x00000000FF000000ULL};
constexpr bitboard BB_RANK_5{0x000000FF00000000ULL};
constexpr bitboard BB_RANK_6{0x0000FF0000000000ULL};
constexpr bitboard BB_RANK_7{0x00FF000000000000ULL};
constexpr bitboard BB_RANK_8{0xFF00000000000000ULL};

// each board file/column
constexpr bitboard BB_FILE_A{0x0101010101010101ULL};
constexpr bitboard BB_FILE_B{0x0202020202020202ULL};
constexpr bitboard BB_FILE_C{0x0404040404040404ULL};
constexpr bitboard BB_FILE_D{0x0808080808080808ULL};
constexpr bitboard BB_FILE_E{0x1010101010101010ULL};
constexpr bitboard BB_FILE_F{0x2020202020202020ULL};
constexpr bitboard BB_FILE_G{0x4040404040404040ULL};
constexpr bitboard BB_FILE_H{0x8080808080808080ULL};

// hex print
inline void hprint_bb(bitboard& b) { printf("0x%016lx", b); }

void print_bb(bitboard b);

#endif // BITBOARD_INCL
