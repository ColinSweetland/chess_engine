#ifndef ZOBRIST_INCL
#define ZOBRIST_INCL

#include "types/bitboard.hpp"
#include "types/pieces.hpp"

#include <cstdint>

// I don't think there is a reason to use uint64_t, since the transposition table definitely won't be
// more than 2^32, however 2^16 seems small, so uint32_t seems right
using zhash_t = uint32_t;

namespace Zobrist
{

// initialize the zobrist has tables
void init();

// get the zobrist value for such a scenario
zhash_t color_piece_on_sq(COLOR c, PIECE p, square sq);
zhash_t black_to_move();
zhash_t castle_right(int cr);
zhash_t ep_square(square sq);

} // namespace Zobrist
#endif // ZOBRIST_INCL