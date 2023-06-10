#include <cinttypes>
#include <cstdint>

#ifndef BITBOARD_INCLUDED
#define BITBOARD_INCLUDED

using bitboard = std::uint64_t;

enum direction
{
    NORTHWEST = 7,
    NORTH = 8,
    NORTHEAST = 9,
    EAST = 1,
    SOUTHEAST = -7,
    SOUTH = -8,
    SOUTHWEST = -9,
    WEST = -1
};

#define PAWN_PUSH_DIR(color) ((color) ? SOUTH : NORTH)

// empty: all zeroes
#define BB_ZERO ((bitboard)0ULL)

// x -> 1 at idx
#define BB_SET(bb, idx) (bb |= (1ULL << (idx)))
// x -> 0 at idx
#define BB_UNSET(bb, idx) (bb &= ~(1ULL << (idx)))
// x -> ~x at idx
#define BB_TOGGLE(bb, idx) (bb ^= (1ULL << (idx)))

// bb at a certain square
// maybe we could use a lookup table later
#define BB_SQ(sq) ((bitboard)(1ULL << (sq)))

// True if bitboard is set at idx else false
#define BB_IS_SET_AT(bb, idx) ((bool)((bb & (1ULL << (idx))) != BB_ZERO))

// "human" coordinate to bb index e.g. (3,1) -> 16
#define RANKFILE_TO_SQ(rank, file) ((rank - 1) * 8 + file - 1)

// index like 56 to 1-8 file or rank num
#define RANK_FROM_SQ(square) ((square / 8) + 1)
#define FILE_FROM_SQ(square) ((square % 8) + 1)

// getting char versions of rank or file, for printing
#define RANK_CHAR_FROM_SQ(sq) (RANK_FROM_SQ(sq) + '1' - 1)
#define FILE_CHAR_FROM_SQ(sq) (FILE_FROM_SQ(sq) + 'a' - 1)

//**** GCC BUILTINS ********
// these can be replaced later if needed
#define BB_LSB(b) (__builtin_ffsll(b) - 1)

#define BB_UNSET_LSB(b) (b &= b - 1)

// we can use this software implementation of PEXT / PDEP later
// https://github.com/zwegner/zp7

// returns a bitboard with the lowest bits corresponding to
// the bits in bb in the positions set in mask
#define BB_PEXT(bb, mask) ((bitboard)_pext_u64(bb, mask))

// returns a bitboard where lowest bits of bb are moved
// into positions set in mask
#define BB_PDEP(bb, mask) ((bitboard)_pdep_u64(bb, mask))

// bit count
#define BB_POPCNT(bb) (__builtin_popcountll(bb))

#define GEN_SHIFT(bb, dir) (((dir) > 0) ? (bb << (dir)) : (bb >> (-(dir))))

// each board row
#define BB_RANK_1 ((bitboard)0x00000000000000FFULL)
#define BB_RANK_2 ((bitboard)0x000000000000FF00ULL)
#define BB_RANK_3 ((bitboard)0x0000000000FF0000ULL)
#define BB_RANK_4 ((bitboard)0x00000000FF000000ULL)
#define BB_RANK_5 ((bitboard)0x000000FF00000000ULL)
#define BB_RANK_6 ((bitboard)0x0000FF0000000000ULL)
#define BB_RANK_7 ((bitboard)0x00FF000000000000ULL)
#define BB_RANK_8 ((bitboard)0xFF00000000000000ULL)

// each board file/column
#define BB_FILE_A ((bitboard)0x0101010101010101ULL)
#define BB_FILE_B ((bitboard)0x0202020202020202ULL)
#define BB_FILE_C ((bitboard)0x0404040404040404ULL)
#define BB_FILE_D ((bitboard)0x0808080808080808ULL)
#define BB_FILE_E ((bitboard)0x1010101010101010ULL)
#define BB_FILE_F ((bitboard)0x2020202020202020ULL)
#define BB_FILE_G ((bitboard)0x4040404040404040ULL)
#define BB_FILE_H ((bitboard)0x8080808080808080ULL)

// hex print
#define hprint_bb(b) (printf("0x%016lx", b))

void print_bb(bitboard b);

#endif
