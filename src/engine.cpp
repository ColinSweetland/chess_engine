#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <sstream>
#include <utility>

#include "bitboard.hpp"
#include "chessmove.hpp"
#include "engine.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"

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

static void perft(Position& pos, int depth, std::vector<uint64_t>& perft_results)
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

int32_t Engine::evaluate(Position pos)
{
    static std::array<int, KING + 1> piece_to_cp_score{0, 0, 100 /*P*/, 300 /*N*/, 300 /*B*/, 500 /*R*/, 900 /*Q*/};

    int32_t score = 0;

    COLOR enemy = static_cast<COLOR>(!pos.side_to_move());

    if (pos.is_game_over())
    {
        return INT32_MIN + 1;
    }

    for (int p = PAWN; p < KING; p++)
    {
        // friendly piece score
        score += piece_to_cp_score[p] * BB_POPCNT(pos.pieces(pos.side_to_move(), static_cast<PIECE>(p)));
        // enemy piece score
        score -= piece_to_cp_score[p] * BB_POPCNT(pos.pieces(enemy, static_cast<PIECE>(p)));
    }

    return score;
}

// Alpha beta search
static scored_move alpha_beta_search(Position pos, int8_t depth, int32_t alpha = INT32_MIN, int32_t beta = INT32_MAX,
                                     bool max = true)
{
    if (depth == 0 || pos.is_game_over())
    {
        int32_t eval = max ? Engine::evaluate(pos) : -Engine::evaluate(pos);
        return std::make_pair(pos.last_move(), eval);
    }
    move_list ml = pos.pseudo_legal_moves();

    // maximizing player
    if (max)
    {
        int32_t   max_eval = INT32_MIN;
        ChessMove max_eval_move;

        for (ChessMove move : ml)
        {
            // skip illegal moves
            if (!pos.try_make_move(move))
                continue;

            scored_move scored = alpha_beta_search(pos, depth - 1, alpha, beta, false);

            // we found a new best move
            if (scored.second > max_eval)
            {
                max_eval_move = move;
                max_eval      = scored.second;
            }

            alpha = std::max(alpha, scored.second);

            // cause cutoff, opponent would never make this move
            if (beta <= alpha)
                break;
            // unmake move from start of loop
            pos.unmake_last();
        }

        return std::make_pair(max_eval_move, max_eval);
    }
    // minimizing player, see comments above
    else
    {
        int32_t   min_eval = INT32_MAX;
        ChessMove min_eval_move;

        for (ChessMove move : ml)
        {
            if (!pos.try_make_move(move))
                continue;

            scored_move scored = alpha_beta_search(pos, depth - 1, alpha, beta, true);

            if (scored.second < min_eval)
            {
                min_eval_move = move;
                min_eval      = scored.second;
            }

            beta = std::min(beta, scored.second);

            if (beta <= alpha)
                break;

            pos.unmake_last();
        }

        return std::make_pair(min_eval_move, min_eval);
    }
}

// Find the best move in a position

ChessMove Engine::best_move(Position pos)
{
    /*
    move_list legal = pos.legal_moves();
    move_list captures;

    for (ChessMove m : legal)
        if (m.is_capture())
            captures.push_back(m);

    return captures.size() == 0 ? legal[std::rand() % legal.size()] : captures[std::rand() % captures.size()];
    */
    return alpha_beta_search(pos, 5).first;
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

        // do nothing if nothing was read
        if (input_buf.empty())
            continue;

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
            // position -> set current position

            size_t i = 3;

            if (cmd_tokens[1] == "fen")
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
            {
                // starter position
                pos = {};
            }
            else
                continue; // ignore invalid input

            // make moves after "moves" token if we had one
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

            std::cout << "bestmove " << best_move(pos) << '\n';
        }
        //*******CUSTOM COMMANDS*********
        else if (cmd_tokens[0] == "printpos")
            std::cout << pos;

        else if (cmd_tokens[0] == "printfen")
            std::cout << pos.FEN() << '\n';

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
