#include "chessmove.hpp"
#include "./types/pieces.hpp"

#include <algorithm>
#include <iostream>

void ChessMove::dump_info() const
{
    std::cout << "MOVE: " << to_str();
    std::cout << "\tMOV P: " << piece_to_str(get_moved_piece());
    std::cout << "\tCAP P: " << piece_to_str(get_captured_piece());
    std::cout << "\t\tPRO P: " << piece_to_str(get_promo_piece());
    std::cout << "\t\tCASTL: " << (is_castle() ? "T" : "F");
    std::cout << "\tSCORE: " << score_for_ordering() << "\n";
}

constexpr int CAPTURE_VALUE   = 40;
constexpr int PROMOTION_VALUE = 80;
constexpr int CASTLE_VALUE    = 20;

// good test positions:

// contains pawns capturing every piece, a promotion, and a castle move
// 1b4k1/P2r4/2P1b3/1q1P1n2/P3P1p1/5P2/6P1/R3K2R w KQ - 0 1

// contains a lot of pieces of varying value attacking a lot of pieces of varying value
// k7/8/8/q1r5/1P2N3/6q1/3q1B2/2Q4K w - - 0 1

// the "score" for the move, a better score means the move will be explored earlier
// Current scheme:
// - Promotion captures ( promo piece > most valuable capture piece > least valuable moved piece )
// - regular promotions ( ordered by promo piece )
// - captures ( most valuable capture piece > least valuable moved piece )
// - castle moves
// - all other moves

int ChessMove::score_for_ordering() const
{
    int score = 0;

    // castles can't be promos or captures so immediate return
    if (is_castle())
        return CASTLE_VALUE;

    if (is_promo())
    {
        score += PROMOTION_VALUE;

        // make promotion piece much more important than capture piece for promo captures
        score += m_aftermove_piece * 10;
    }

    if (is_capture())
    {
        score += CAPTURE_VALUE;

        // order first by capture piece (best piece first)
        // then by moved piece (worst piece first)
        score += m_cap_piece * 5;
        score -= m_moved_piece;
    }

    return score;
}

void order_moves(move_list& ml)
{
    std::sort(ml.begin(), ml.end(), [](auto m1, auto m2) { return m1.score_for_ordering() > m2.score_for_ordering(); });
}
