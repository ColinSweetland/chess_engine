#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <sstream>

#include "chessmove.hpp"
#include "engine.hpp"
#include "evaluate.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"
#include "util.hpp"

// Create a chessmove on pos with a string representing a move (in format UCI uses)
static ChessMove UCI_move(Position pos, str move_string)
{
    const int origin_file = move_string[0] - 'a' + 1;
    const int origin_rank = move_string[1] - '1' + 1;
    const int dest_file   = move_string[2] - 'a' + 1;
    const int dest_rank   = move_string[3] - '1' + 1;

    const int promo_rank = PAWN_PROMO_RANK(pos.side_to_move());

    const square origin = rankfile_to_sq(origin_rank, origin_file);
    assert(origin >= 0 && origin <= 63);
    const square dest = rankfile_to_sq(dest_rank, dest_file);
    assert(dest >= 0 && dest <= 63);

    const PIECE moved_p = pos.piece_at_sq(origin);

    PIECE capture_p = pos.piece_at_sq(dest);

    PIECE promo_p = NO_PIECE;

    // pawn special info
    if (moved_p == PAWN)
    {
        // En passante capture
        if (dest_file != origin_file && capture_p == NO_PIECE)
            capture_p = EN_PASSANTE;

        // double push
        else if (dest == origin + PAWN_PUSH_DIR(pos.side_to_move()) * 2)
            return {origin, dest, ChessMove::flags::DOUBLE_PUSH};

        // promo
        else if (dest_rank == promo_rank)
            promo_p = char_to_piece.at(move_string[4]);
    }
    else if (moved_p == KING)
    {
        // check if castle
        if (origin_file == 5 /*e*/)
        {
            // If the king ever moves 2 files, it's a castle move
            if (dest_file == 3 /*c*/)
                return {origin, dest, ChessMove::flags::QUEENSIDE_CASTLE};

            else if (dest_file == 7 /*g*/)
                return {origin, dest, ChessMove::flags::KINGSIDE_CASTLE};
        }
    }

    return {origin, dest, ChessMove::makeflag(capture_p, promo_p)};
}

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

const size_t  MAX_UCI_INPUT_SIZE   = 1024;
const uint8_t DEFAULT_SEARCH_DEPTH = 4;

void Engine::uci_loop()
{
    Position pos;

    str input_buf(MAX_UCI_INPUT_SIZE, ' ');

    str token;

    std::vector<str> cmd_tokens;

    while (1)
    {
        // get one line (command) and store it into input buf
        // then make a stream 'line' with the input buf
        getline(std::cin, input_buf);
        std::istringstream line{input_buf};

        // collect tokens from the line into vector cmd_tokens
        cmd_tokens.clear();
        while (line >> token)
            cmd_tokens.push_back(token);

        // if no tokens were read, do nothing
        if (cmd_tokens.size() == 0)
            continue;

        // parse the commands
        if (cmd_tokens[0] == "uci")
        {
            // uci command -> identify engine with id
            //             -> TODO: set options with option
            //             -> uciok to verify using uci
            std::cout << "id name test_engine\n"
                      << "id author Colin Sweetland\n"
                      << "uciok\n";
        }
        else if (cmd_tokens[0] == "quit")
        {
            // quit command -> exit
            std::cout << "info string receieved quit command. exiting...\n";
            break;
        }
        else if (cmd_tokens[0] == "isready")
        {
            // isready -> used for sync with GUI
            //         -> always respond "readyok"
            std::cout << "readyok\n";
        }
        else if (cmd_tokens[0] == "setoption")
        {
            // setoption name <id> value <x> -> set an engine option (we have none right now)
            std::cout << "info string options not implemented\n";
        }
        else if (cmd_tokens[0] == "position" && cmd_tokens.size() >= 2)
        {
            // position -> set current position

            size_t i = 3;

            // fen strings should have 6 tokens -> position fen (6 tokens) = 8
            if (cmd_tokens[1] == "fen" && cmd_tokens.size() >= 8)
            {
                str fen;
                // collect tokens until "moves" or out of tokens
                for (i = 2; i < cmd_tokens.size(); i++)
                {
                    if (cmd_tokens[i] == "moves")
                    {
                        i++; // consume 'moves' token
                        break;
                    }
                    // our fen parser can handle space at the end
                    fen += cmd_tokens[i];
                    fen += ' ';
                }

                pos = {fen};
            }
            else if (cmd_tokens[1] == "startpos")
                pos = {};
            else
                continue; // ignore invalid input

            // make moves after "moves" token if we have one
            for (; i < cmd_tokens.size(); i++)
            {
                ChessMove uci_m = UCI_move(pos, cmd_tokens[i]);
                pos.make_move(uci_m);
            }
        }
        else if (cmd_tokens[0] == "ucinewgame")
        {
            // ucinewgame -> next position command will be a new game
            //            -> reset necessary state (we have none)
            std::cout << "info string got command 'ucinewgame' but nothing to do\n";
        }
        else if (cmd_tokens[0] == "go")
        {
            // go -> many different commands depending on subcommands
            //    -> mainly search for next best move

            uint8_t depth = DEFAULT_SEARCH_DEPTH;

            for (size_t i = 0; i < cmd_tokens.size() - 1; i++)
                if (cmd_tokens[i] == "depth")
                    depth = std::stoi(cmd_tokens[++i]);

            std::cout << "bestmove " << best_move(pos, depth) << '\n';
        }
        //*******CUSTOM COMMANDS*********
        else if (cmd_tokens[0] == "printpos")
            std::cout << pos;

        else if (cmd_tokens[0] == "printfen")
            std::cout << pos.FEN() << '\n';
        else if (cmd_tokens[0] == "printeval")
            std::cout << "EVAL: " << evaluate(pos) << '\n';

        else if (cmd_tokens[0] == "perft")
        {
            int perft_depth = std::stoi(cmd_tokens[1]);

            perft_report(pos, perft_depth);
        }
        else if (cmd_tokens[0] == "perftdiv")
        {
            int perft_depth = std::stoi(cmd_tokens[1]);

            perft_report_divided(pos, perft_depth);
        }

    } // end while
}
