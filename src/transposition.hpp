#ifndef TRANSPOSITION_INCL
#define TRANSPOSITION_INCL

#include "chessmove.hpp"
#include "evaluate.hpp"
#include "zobrist.hpp"

using Engine::centipawn;

namespace tt
{

void init();

enum class NODE_TYPE
{
    PV,     // Exact: Principal Variation
    ALL,    // Upper bound: all moves were searched
    CUT,    // Lower bound: a cutoff was found
    INVALID // Invalid: nothing is stored here.
};

// entry of a transposition table
struct entry
{
    ChessMove best_move = {};
    centipawn value     = 0;
    NODE_TYPE node_type = NODE_TYPE::INVALID;
    int       depth     = 0;
    zhash_t   full_hash = 0;
};

inline bool valid_entry(entry e) { return e.node_type != NODE_TYPE::INVALID; }

void store(entry e);

entry lookup(zhash_t pos_hash);

} // namespace tt

#endif // TRANSPOSITION_INCL