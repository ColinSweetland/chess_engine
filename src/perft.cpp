#include <chrono>
#include <iomanip>

#include "perft.hpp"
#include "util.hpp"

static void perft(Position& pos, int depth, std::vector<std::uint64_t>& perft_results)
{
    if (depth == 0)
    {
        perft_results.back() = 1;
        return;
    }

    // we won't use legal_moves here because we end up
    // wasting a unmake
    move_list pl_moves = pos.pseudo_legal_moves();

    perft_results[depth - 1] += pl_moves.size();

    for (ChessMove move : pl_moves)
    {
        if (pos.try_make_move(move))
        {

            perft(pos, depth - 1, perft_results);
            pos.unmake_last();
        }
        else
        {
            // there was an illegal move, it doesn't count
            perft_results[depth - 1] -= 1;
        }
    }
}

void Engine::perft_report(Position& pos, int depth)
{
    assert(depth >= 0);

    std::vector<std::uint64_t> perft_results(depth + 1, 0);

    const auto start = std::chrono::steady_clock::now();

    perft(pos, depth, perft_results);

    const auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsed_sec = end - start;

    std::cout << "\nDEPTH | NODES \n------------------\n";

    std::cout << std::left;
    for (int i = depth; i >= 0; i--)
    {
        std::cout << std::setw(6) << depth - i << "| " << util::pretty_int(perft_results.at(i)) << '\n';
    }
    // restore default
    std::cout << std::right;

    const std::uint64_t nps = perft_results.front() / elapsed_sec.count();

    std::cout << "\nTime elapsed: " << elapsed_sec.count() << "s\t(" << util::pretty_int(nps) << " Nodes/sec)\n\n";
}

void Engine::perft_report_divided(Position& pos, int depth)
{
    assert(depth >= 1);

    const auto start = std::chrono::steady_clock::now();

    move_list ml = pos.pseudo_legal_moves();

    // nodes are only leaves (depth 0)
    std::uint64_t total_nodes = 0;

    for (ChessMove move : ml)
    {
        if (pos.try_make_move(move))
        {
            std::vector<uint64_t> perft_results(depth, 0);

            perft(pos, depth - 1, perft_results);

            std::cout << move << ": " << perft_results.front();

            std::cout << '\n';

            total_nodes += perft_results.front();

            pos.unmake_last();
        }
    }

    const auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsed_sec = end - start;

    const std::uint64_t nps = total_nodes / elapsed_sec.count();

    std::cout << "\nNodes searched: " << util::pretty_int(total_nodes) << '\n';
    std::cout << "Time elapsed: " << elapsed_sec.count() << "s "
              << "(" << util::pretty_int(nps) << " Nodes/sec)\n";
}