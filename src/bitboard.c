#include <inttypes.h>
#include <stdio.h>
#include "bitboard.h"

void print_bb(bitboard bb)
{
    //space
    printf("\n");

    /* iterate through ranks (rows) of the board
    ranks are 1 indexed, and we iterate backwards, because
    squares are starting at bottom left (0) to top right (63)
    this fits the standard way chess boards are labeled (from white side) */
    for (int rank = 8; rank > 0; rank--)
    {
        // rank number label
        printf("%d\t", rank);     

        // iterate through files (columns) of the board
        // files are 1 indexed   
        for (int file = 1; file <= 8; file++)
        {
            int square = RANKFILE_TO_SQ(rank, file);

            printf(" %c", BB_IS_SET_AT(bb, square) ? 'X' : '.');
        }

        printf("\n");
    }

    // file info + space
    printf("\n\t a b c d e f g h\n\n");
}
