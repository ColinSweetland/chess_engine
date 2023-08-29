#include "chessmove.hpp"
#include "./types/pieces.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

constexpr int CAPTURE_VALUE   = 40;
constexpr int PROMOTION_VALUE = 80;
constexpr int CASTLE_VALUE    = 20;

// the "score" for the move, a better score means the move will be explored earlier
// Current scheme:
// - Promotion captures ( promo piece > most valuable capture piece > least valuable moved piece )
// - regular promotions ( ordered by promo piece )
// - captures ( most valuable capture piece > least valuable moved piece )
// - castle moves
// - all other moves

int score_for_ordering(const ChessMove& move, const ChessMove& tt_best_move)
{
    int score = 0;

    if (move == tt_best_move)
    {
        // std::cout << "move matches tt best!!!\n";
        return std::numeric_limits<int>::max();
    }

    // castles can't be promos or captures so immediate return
    if (move.is_castle())
        return CASTLE_VALUE;

    if (move.is_promo())
    {
        score += PROMOTION_VALUE;

        // make promotion piece much more important than capture piece for promo captures
        score += move.get_after_move_piece() * 10;
    }

    if (move.is_capture())
    {
        score += CAPTURE_VALUE;

        // order first by capture piece (best piece first)
        // then by moved piece (worst piece first)
        score += move.get_captured_piece() * 5;
        score -= move.get_moved_piece();
    }

    return score;
}

// NOTE: this may be slow, since I think sort will have to call score_for_ordering multiple times for the same move.
// may be a place to improve.

void order_moves(move_list& ml, const ChessMove& tt_best_move)
{
    std::sort(ml.begin(), ml.end(), [&](auto m1, auto m2) {
        return score_for_ordering(m1, tt_best_move) > score_for_ordering(m2, tt_best_move);
    });
}

void ChessMove::dump_info() const
{
    std::cout << "MOVE: " << to_str();
    std::cout << "\tMOV P: " << piece_to_str(get_moved_piece());
    std::cout << "\tCAP P: " << piece_to_str(get_captured_piece());
    std::cout << "\t\tPRO P: " << piece_to_str(get_promo_piece());
    std::cout << "\t\tCASTL: " << (is_castle() ? "T" : "F");
    std::cout << "\tSCORE: " << score_for_ordering(*this, ChessMove{}) << "\n";
}
