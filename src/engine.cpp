#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include "engine.hpp"
#include "movegen.hpp"
#include "types.hpp"

void perft(Position& p, int depth, std::vector<uint64_t>& perft_results)
{
    if (depth == 0)
    {
        perft_results.back() = 1;
        return;
    }

    move_list pl_moves;

    int n_moves = p.pseudo_legal_moves(pl_moves);

    // printf("SIZEOF RES %zu\n", perft_results.size());
    // printf("DEPTH: %d\n", depth);
    // exit(1);

    perft_results[depth - 1] += n_moves;

    for (int i = 0; i < n_moves; i++)
    {
        if (p.try_make_move(pl_moves[i]))
        {
            perft(p, depth - 1, perft_results);
            p.unmake_last();
        }
        else
        {
            // there was an illegal move, it doesn't count
            perft_results[depth - 1] -= 1;
        }
    }
}

void perft_report(Position& pos, int depth)
{
    assert(depth >= 0);

    std::vector<uint64_t> perft_results(depth + 1, 0);

    perft(pos, depth, perft_results);

    printf("DEPTH | NODES \n------------------\n");
    for (int i = depth; i >= 0; i--)
    {
        printf("%-6d| %-15lu\n", depth - i, perft_results.at(i));
    }
    printf("\n");
}

void perft_report_divided(Position& pos, int depth, bool print_fen)
{
    assert(depth >= 1);

    move_list ml{};

    int move_count = pos.pseudo_legal_moves(ml);

    // nodes are only leaves (depth 0)
    int total_nodes = 0;

    for (int i = 0; i < move_count; i++)
    {
        if (pos.try_make_move(ml[i]))
        {
            std::vector<uint64_t> perft_results(depth, 0);

            perft(pos, depth - 1, perft_results);

            std::cout << ml[i] << ": " << perft_results.front();

            if (print_fen)
                std::cout << '\t' << pos.FEN();

            std::cout << '\n';

            total_nodes += perft_results.front();

            pos.unmake_last();
        }
    }

    std::cout << "\nNodes searched: " << total_nodes << '\n';
}
