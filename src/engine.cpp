#include <cassert>

#include "engine.h"

uint64_t perft(Position& p, int depth)
{
    move_list pl_moves;

    std::uint64_t nodes = 0;

    if (depth == 0)
        return 1ULL;

    int n_moves = p.pseudo_legal_moves(pl_moves);

    for (int i = 0; i < n_moves; i++)
    {
        p.make_move(pl_moves[i]);

        if (!p.is_check(static_cast<COLOR>(!p.side_to_move())))
            nodes += perft(p, depth - 1);

        p.unmake_last();
    }

    return nodes;
}

void perft_report(Position& p, int depth)
{
    assert(depth >= 1);
    for (int i = 1; i <= depth; i++)
    {
        std::cout << "depth " << i << ": " << perft(p, i) << "\n";
    }
}
