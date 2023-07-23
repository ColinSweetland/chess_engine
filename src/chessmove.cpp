#include "chessmove.hpp"

void ChessMove::dump_info() const
{
    std::cout << "MOVE: " << to_str();
    std::cout << "\tMOV P: " << piece_to_str.at(get_moved_piece());
    std::cout << "\tCAP P: " << piece_to_str.at(get_captured_piece());
    std::cout << "\t\tPRO P: " << piece_to_str.at(get_promo_piece());
    std::cout << "\t\tCASTL: " << (is_castle() ? "T" : "F") << "\n";
}