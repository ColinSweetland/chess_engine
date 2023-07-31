#include <array>
#include <cassert>
#include <cstdint>
#include <sstream>

#include "bitboard.hpp"
#include "chessmove.hpp"
#include "engine.hpp"
#include "evaluate.hpp"
#include "movegen.hpp"
#include "perft.hpp"
#include "position.hpp"
#include "types.hpp"
#include "util.hpp"

static bool interactive = false;

void Engine::set_interactive() { interactive = true; }

// send "info string" message - omit "info string" if interactive
static void send_info(str message)
{
    if (!interactive)
        std::cout << "info string ";
    std::cout << message << '\n';
}

// uci command -> identify engine with id
//             -> TODO: set options with option cmd
//             -> uciok cmd to verify using uci
static void uci_cmd()
{
    std::cout << "id name test_engine\n"
              << "id author Colin Sweetland\n"
              << "uciok\n";
}

const uint8_t DEFAULT_SEARCH_DEPTH = 4;

// go -> find best move
static void go_cmd(Position& pos, std::vector<str>& tokens)
{
    uint8_t depth = DEFAULT_SEARCH_DEPTH;

    // search for 'depth' subcommand (others not supported right now)
    // stop 1 short of the end because depth command needs an argument
    for (size_t i = 0; i < tokens.size() - 1; i++)
        if (tokens[i] == "depth")
            depth = std::stoi(tokens[++i]);

    std::cout << "bestmove " << Engine::best_move(pos, depth) << '\n';
}

// Create a chessmove on pos with a string representing a move (in format UCI uses)
static const ChessMove UCI_move(Position& pos, str move_string)
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

    PIECE after_move_p = moved_p;

    // pawn special info
    if (moved_p == PAWN)
    {
        // En passante capture
        if (dest_file != origin_file && capture_p == NO_PIECE)
            capture_p = EN_PASSANTE;
        // promo
        else if (dest_rank == promo_rank)
            after_move_p = char_to_piece.at(move_string[4]);
    }

    return {moved_p, after_move_p, origin, dest, capture_p};
}

// position -> set current position
static void position_cmd(Position& pos, std::vector<str>& tokens)
{
    size_t i = 3;

    // fen strings should have 6 tokens (space seperated fields)
    // -> position fen (6 tokens) = 8
    if (tokens.size() >= 8 && tokens[1] == "fen")
    {
        str fen;
        // collect tokens until "moves" or out of tokens
        for (i = 2; i < tokens.size(); i++)
        {
            if (tokens[i] == "moves")
            {
                i++; // consume 'moves' token
                break;
            }
            // our fen parser can handle space at the end
            fen += tokens[i];
            fen += ' ';
        }

        pos = {fen};
    }
    else if (tokens.size() >= 2 && tokens[1] == "startpos")
        pos = {};
    else
        return; // ignore invalid input -> noop

    // make moves after "moves" token if we have one
    for (; i < tokens.size(); i++)
    {
        ChessMove uci_m = UCI_move(pos, tokens[i]);
        pos.make_move(uci_m);
    }

    // interactive mode feature: auto print the position
    if (interactive)
        std::cout << pos;
}

const size_t MAX_UCI_INPUT_SIZE = 1024;

void Engine::uci_loop()
{
    Position pos;

    str input_buf(MAX_UCI_INPUT_SIZE, ' ');

    str token;

    std::vector<str> cmd_tokens;

    if (interactive)
        send_info("running interactively");

    for (bool quit = false; !quit;)
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

        // now parse the commands

        if (cmd_tokens[0] == "uci")
            uci_cmd();

        else if (cmd_tokens[0] == "quit")
            quit = true;

        //  sync with GUI
        else if (cmd_tokens[0] == "isready")
            std::cout << "readyok\n";

        // setoption name <id> value <x> -> set an engine option (we have none right now)
        else if (cmd_tokens[0] == "setoption")
            send_info("options not implemented");

        else if (cmd_tokens[0] == "position")
            position_cmd(pos, cmd_tokens);

        // ucinewgame -> next position command will be a new game
        //            -> reset necessary state (e.g. transposition table) (we have none right now)
        else if (cmd_tokens[0] == "ucinewgame")
            send_info("got command 'ucinewgame' but nothing to do");

        else if (cmd_tokens[0] == "go")
            go_cmd(pos, cmd_tokens);

        //*******CUSTOM COMMANDS*********
        else if (cmd_tokens[0] == "printpos")
            std::cout << pos;

        else if (cmd_tokens[0] == "printfen")
            std::cout << pos.FEN() << '\n';

        else if (cmd_tokens[0] == "printeval")
        {
            auto psl = pos.pseudo_legal_moves();
            std::cout << "EVAL: " << Engine::evaluate(pos, psl) << '\n';
        }
        else if (cmd_tokens[0] == "make")
        {
            pos.make_move(UCI_move(pos, cmd_tokens[1]));
            std::cout << pos;
        }
        else if (cmd_tokens[0] == "unmake")
        {
            pos.unmake_last();
            std::cout << pos;
        }
        else if (cmd_tokens[0] == "checkmask")
        {
            print_bb(create_check_mask(pos));
        }
        else if (cmd_tokens[0] == "checkers")
        {
            print_bb(pos.get_checkers_bb());
        }
        else if (cmd_tokens[0] == "dumphist")
            pos.dump_move_history();

        else if (cmd_tokens[0] == "lminfo")
        {
            auto moves = pos.legal_moves();
            order_moves(moves);
            for (auto m : moves)
                m.dump_info();
        }
        else if (cmd_tokens[0] == "plminfo")
        {
            auto moves = pos.pseudo_legal_moves();
            order_moves(moves);
            for (auto m : moves)
                m.dump_info();
        }
        else if (cmd_tokens[0] == "perft")
        {
            int perft_depth = std::stoi(cmd_tokens[1]);

            Engine::perft_report(pos, perft_depth);
        }
        else if (cmd_tokens[0] == "perftdiv")
        {
            int perft_depth = std::stoi(cmd_tokens[1]);

            Engine::perft_report_divided(pos, perft_depth);
        }

    } // end while
}
