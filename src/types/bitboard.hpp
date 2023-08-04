#ifndef BITBOARD_INCL
#define BITBOARD_INCL

#include "../util.hpp"

#include <cassert>
#include <cstdint>
#include <immintrin.h>
#include <string>

/* This file contains the definition and functions for the bitboard type,
 * and also square and direction type, since they are closely related
 */

using bitboard = std::uint64_t;

// usually useful as index into a bitboard
using square = std::int8_t;

// direction on the board, from white's point of view
enum DIR : int
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

inline bool is_valid(square sq) { return sq >= 0 && sq <= 63; }

// empty: all zeroes
constexpr bitboard BB_ZERO = {0ULL};

// bb at a certain square
// maybe we could use a lookup table later
constexpr bitboard bb_from_sq(square sq)
{
    assert(is_valid(sq));
    return (1ULL << (sq));
}

// x -> 1 at idx
inline void bb_set_sq(bitboard& bb, square sq) { bb |= bb_from_sq(sq); }

// x -> 0 at idx
inline void bb_unset_sq(bitboard& bb, square sq) { bb &= ~bb_from_sq(sq); }

// x -> ~x at idx
inline void bb_toggle_sq(bitboard& bb, square sq) { bb ^= bb_from_sq(sq); }

// True if bitboard is set at idx else false
inline bool bb_is_set_at_sq(const bitboard& bb, square sq) { return bb & bb_from_sq(sq); }

// "human" coordinate to bb index e.g. (3,1) -> 16
constexpr square rf_to_sq(int rank, int file) { return (rank - 1) * 8 + file - 1; }

constexpr int file_num(square sq) { return (sq % 8) + 1; }
constexpr int rank_num(square sq) { return (sq / 8) + 1; }

// getting char versions of rank or file, for printing
constexpr char rank_char(square sq) { return static_cast<char>(rank_num(sq) + '1' - 1); }
constexpr char file_char(square sq) { return static_cast<char>(file_num(sq) + 'a' - 1); }

inline std::string sq_str(square sq) { return {file_char(sq), rank_char(sq)}; }

// squares on bottom becomes mirrored vertically, so sq 0 becomes 56, sq 15 becomes 55 etc.
constexpr square mirror_vertically(square sq) { return sq ^ 56; }

//**** GCC BUILTINS ********
// these can be replaced later if needed
constexpr square lsb(const bitboard b) { return __builtin_ffsll(b) - 1; }

inline void unset_lsb(bitboard& b) { b &= b - 1; }

inline square pop_lsb(bitboard& b)
{
    auto ret = lsb(b);
    unset_lsb(b);
    return ret;
}

// we can use this software implementation of PEXT / PDEP later
// https://github.com/zwegner/zp7

// returns a bitboard with the lowest bits corresponding to
// the bits in bb in the positions set in mask
constexpr bitboard pext(const bitboard bb, const bitboard mask) { return _pext_u64(bb, mask); }

// returns a bitboard where lowest bits of bb are moved
// into positions set in mask
constexpr bitboard pdep(const bitboard bb, const bitboard mask) { return _pdep_u64(bb, mask); }

// population count: count of set bits
constexpr int popcnt(const bitboard bb) { return __builtin_popcountll(bb); }

// add directions with + operator and no static cast
constexpr DIR operator+(DIR l, DIR r) { return static_cast<DIR>(util::to_underlying(l) + util::to_underlying(r)); }

// use unary - to flip direction (e.g. north to south)
constexpr DIR operator-(DIR d) { return static_cast<DIR>(-util::to_underlying(d)); };

// constexpr DIR PAWN_PUSH_DIR(COLOR c) { return c ? SOUTH : NORTH; }

// shift and swap direction properly when negative
constexpr bitboard gen_shift(const bitboard bb, const DIR dir) { return dir > 0 ? (bb << dir) : (bb >> (-dir)); }

// each board row
constexpr bitboard BB_RANK_1 = {0x00000000000000FFULL};
constexpr bitboard BB_RANK_2 = {0x000000000000FF00ULL};
constexpr bitboard BB_RANK_3 = {0x0000000000FF0000ULL};
constexpr bitboard BB_RANK_4 = {0x00000000FF000000ULL};
constexpr bitboard BB_RANK_5 = {0x000000FF00000000ULL};
constexpr bitboard BB_RANK_6 = {0x0000FF0000000000ULL};
constexpr bitboard BB_RANK_7 = {0x00FF000000000000ULL};
constexpr bitboard BB_RANK_8 = {0xFF00000000000000ULL};

// each board file/column
constexpr bitboard BB_FILE_A = {0x0101010101010101ULL};
constexpr bitboard BB_FILE_B = {0x0202020202020202ULL};
constexpr bitboard BB_FILE_C = {0x0404040404040404ULL};
constexpr bitboard BB_FILE_D = {0x0808080808080808ULL};
constexpr bitboard BB_FILE_E = {0x1010101010101010ULL};
constexpr bitboard BB_FILE_F = {0x2020202020202020ULL};
constexpr bitboard BB_FILE_G = {0x4040404040404040ULL};
constexpr bitboard BB_FILE_H = {0x8080808080808080ULL};

// hex print
inline void hprint_bb(bitboard b) { printf("0x%016lx", b); }

void print_bb(bitboard b);

#endif // BITBOARD_INCL
