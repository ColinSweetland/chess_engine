#include <iostream>

#include "bitboard.hpp"

void print_bb(bitboard bb)
{
    // space
    std::cout << "\n";

    /* ranks and files are 1 indexed, we iterate backwards, because
    squares are starting at bottom left (0) to top right (63)
    this is the standard way chess boards are labeled (from white side) */
    for (int rank = 8; rank > 0; rank--)
    {
        // rank number label
        std::cout << rank << "\t";

        for (int file = 1; file <= 8; file++)
        {
            int square = rankfile_to_sq(rank, file);

            // print X for 1, . for 0
            std::cout << (BB_IS_SET_AT(bb, square) ? " X" : " .");
        }

        std::cout << "\n";
    }

    // file label + space
    std::cout << "\n\t a b c d e f g h\n\n";
}
