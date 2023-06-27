#include <cassert>
#include <sstream>

#include "engine.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"

static void perft(Position& pos, int depth, std::vector<uint64_t>& perft_results)
{
    if (depth == 0)
    {
        perft_results.back() = 1;
        return;
    }

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

    std::vector<uint64_t> perft_results(depth + 1, 0);

    perft(pos, depth, perft_results);

    printf("DEPTH | NODES \n------------------\n");
    for (int i = depth; i >= 0; i--)
    {
        printf("%-6d| %-15lu\n", depth - i, perft_results.at(i));
    }
    printf("\n");
}

void Engine::perft_report_divided(Position& pos, int depth, bool print_fen)
{
    assert(depth >= 1);

    move_list ml = pos.pseudo_legal_moves();

    // nodes are only leaves (depth 0)
    int total_nodes = 0;

    for (ChessMove move : ml)
    {
        if (pos.try_make_move(move))
        {
            std::vector<uint64_t> perft_results(depth, 0);

            perft(pos, depth - 1, perft_results);

            std::cout << move << ": " << perft_results.front();

            if (print_fen)
                std::cout << '\t' << pos.FEN();

            std::cout << '\n';

            total_nodes += perft_results.front();

            pos.unmake_last();
        }
    }

    std::cout << "\nNodes searched: " << total_nodes << '\n';
}

const size_t MAX_UCI_INPUT_SIZE = 1024;

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
        {
            cmd_tokens.push_back(token);
        }

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
        else if (cmd_tokens[0] == "position")
        {
            // position            -> set current position
            // second arg fen      -> set position to fen
            // second arg startpos -> set to startpos
            if (cmd_tokens[1] == "fen")
            {
                str fen;

                // collect tokens until "moves" or out of tokens
                for (size_t i = 2; i < cmd_tokens.size(); i++)
                {
                    if (cmd_tokens[i] == "moves")
                        break;

                    // luckily our fen parser can handle space at the end
                    fen += cmd_tokens[i];
                    fen += ' ';
                }

                // TODO: Initialize with moves

                pos = {fen};
            }
            else if (cmd_tokens[1] == "startpos")
            {
                // starter position
                pos = {};
            }
        }
        else if (cmd_tokens[0] == "ucinewgame")
        {
            // ucinewgame -> next position command will be a new game
            //            -> reset necessary state (we have none)
            std::cout << "info string got command 'ucinewgame' but nothing to do\n";
        }
        //*******CUSTOM COMMANDS*********
        else if (cmd_tokens[0] == "printpos")
        {
            std::cout << pos;
        }
        else if (cmd_tokens[0] == "printfen")
        {
            std::cout << pos.FEN() << '\n';
        }
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
