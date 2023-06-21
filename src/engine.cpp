#include <cassert>

#include "engine.h"
#include "movegen.h"
#include "types.h"

uint64_t perft(Position& p, int depth)
{
    move_list pl_moves;

    std::uint64_t nodes = 0;

    if (depth == 0)
        return 1;

    int n_moves = p.pseudo_legal_moves(pl_moves);

    for (int i = 0; i < n_moves; i++)
    {
        if (p.move_is_legal(pl_moves[i]))
            nodes += perft(p, depth - 1);
    }

    return nodes;
}

void perft_report(Position& p, int depth)
{
    assert(depth >= 1);
    for (int i = 1; i <= depth; i++)
    {
        printf("DEPTH: %-5d NODES : %-15lu\n\n", i, perft(p, i));
    }
}

void perft_report_divided(Position& p, int depth)
{
    assert(depth >= 1);

    move_list ml{};

    int move_count = p.pseudo_legal_moves(ml);

    int total_nodes  = 0;
    int nodes_at_pos = 0;

    for (int ml_index = 0; ml_index < move_count; ml_index++)
    {
        if (!p.move_is_legal(ml[ml_index]))
            continue;

        std::cout << ml.at(ml_index);

        p.make_move(ml[ml_index]);
        nodes_at_pos = perft(p, depth - 1);
        total_nodes += nodes_at_pos;

        printf(": %d\n", nodes_at_pos);
        p.unmake_last();
    }

    printf("\nnodes searched : %-15d\n", total_nodes);
}
