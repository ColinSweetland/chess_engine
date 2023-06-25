#include <cassert>

#include "engine.hpp"
#include "movegen.hpp"
#include "types.hpp"

uint64_t perft(Position& p, int depth)
{
    move_list pl_moves;

    std::uint64_t nodes = 0;

    if (depth == 0)
        return 1ULL;

    int n_moves = p.pseudo_legal_moves(pl_moves);

    for (int i = 0; i < n_moves; i++)
    {
        if (p.move_is_legal(pl_moves[i]))
        {
            // NOTE this is unefficient (we make and unmake twice)
            // maybe make a method like
            // bool try_make_move() -> try making pseudolegal move, returns if successful
            p.make_move(pl_moves[i]);
            nodes += perft(p, depth - 1);
            p.unmake_last();
        }
    }

    return nodes;
}

void perft_report(Position& pos, int depth)
{
    assert(depth >= 1);
    for (int i = 1; i <= depth; i++)
    {
        printf("DEPTH: %-5d NODES : %-15lu\n\n", i, perft(pos, i));
    }
}

void perft_report_divided(Position& pos, int depth)
{
    assert(depth >= 1);

    move_list ml{};

    int move_count = pos.pseudo_legal_moves(ml);

    int total_nodes  = 0;
    int nodes_at_pos = 0;

    for (int ml_index = 0; ml_index < move_count; ml_index++)
    {
        if (!pos.move_is_legal(ml[ml_index]))
            continue;

        pos.make_move(ml[ml_index]);

        nodes_at_pos = perft(pos, depth - 1);

        std::cout << ml.at(ml_index) << ": ";
        std::cout << nodes_at_pos << "\n"; // << pos.FEN();

        // std::cout << pos;
        // std::cout << pos.FEN();

        pos.unmake_last();
        total_nodes += nodes_at_pos;
    }

    printf("\nNodes searched: %-15d\n", total_nodes);
}
