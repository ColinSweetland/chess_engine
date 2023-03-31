#include <inttypes.h>

typedef uint64_t bitboard;

void print_bitboard(bitboard b);

// set the bit at the index in b to 1
void set_bboard_index(bitboard *b, int index);