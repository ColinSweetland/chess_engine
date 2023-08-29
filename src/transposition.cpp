#include "transposition.hpp"

#include <math.h>

// must be power of 2 size
static constexpr size_t tt_size       = (1 << 22);
static constexpr size_t tt_index_mask = tt_size - 1;

static tt::entry transposition_table[tt_size];

// fill transposition table with invalid entries
void tt::init()
{
    for (size_t i = 0; i < tt_size; i++)
        transposition_table[i] = entry{};
}

void tt::store(tt::entry entry)
{
    size_t tt_index = tt_index_mask & entry.full_hash;

    // currently using "replace always" scheme, but their are many others worth considering
    // https://www.chessprogramming.org/Transposition_Table#Replacement_Strategies
    transposition_table[tt_index] = entry;
}

tt::entry tt::lookup(zhash_t pos_hash)
{
    tt::entry result = transposition_table[tt_index_mask & pos_hash];

    // the index collided, but the full hash didn't.
    if (pos_hash != result.full_hash)
        return entry{};

    return result;
}
