#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include "bitboard.h"

// bitboards are 64 bits, each bit represents 1 of 64 squares on a chessboard
typedef uint64_t bitboard;

void print_bitboard(bitboard b)
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
        for (int file = 0; file < 8; file++)
        {

            
            int square = (rank - 1)*8 + file;

            if ((b & (bitboard) pow(2,square)) > 0) 
            {
                printf(" 1");
            } 
            else
            {
                printf(" 0");
            }
            
            
        }

        printf("\n");
    }

    // file info + space
    printf("\n\t a b c d e f g h\n\n");
}

// set a bit at the specified index 
void set_bboard_index(bitboard *b, int index)
{
    // a square is the bit represented by 2 to the power of index
    *b |= (bitboard) pow(2, index);
}

int is_set_at_index(bitboard b, int index) 
{
    return (b & (bitboard) pow(2, index)) > 0;
}