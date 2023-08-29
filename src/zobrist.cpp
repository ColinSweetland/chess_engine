#include "zobrist.hpp"
#include "types/bitboard.hpp"

#include <random>

// One number for each color, piece combo per square (12 * 64) = 768
// One number to indicate side to move is black  (769)
// 16 numbers to indicate castle rights        (785)
// Eight number to indicate en passante file     (793)

// note: technically we don't need number for pawn on backranks, TODO: maybe fix
constexpr size_t HASH_VALUE_LOOKUP_SIZE = 793;

zhash_t hash_value_lookup[HASH_VALUE_LOOKUP_SIZE] = {};

void Zobrist::init()
{
    // mersenne twister engine
    std::mt19937 random_engine;
    random_engine.seed(675022132);

    // simply fill hash_value_lookup with random numbers
    for (size_t i = 0; i < HASH_VALUE_LOOKUP_SIZE; i++)
        hash_value_lookup[i] = random_engine();
}

zhash_t Zobrist::color_piece_on_sq(COLOR c, PIECE p, square sq)
{
    assert(c == WHITE || c == BLACK);
    assert(p >= PAWN && p <= KING);

    // First 6 * 64 are for white, second 6 * 64 are for black
    size_t color_offset = (6 * 64) * c;
    size_t piece_offset = (p - 1) * 64;

    return hash_value_lookup[color_offset + piece_offset + sq];
};

zhash_t Zobrist::black_to_move() { return hash_value_lookup[768]; }

zhash_t Zobrist::castle_right(int cr)
{
    assert(cr >= 0 && cr <= 15);
    return hash_value_lookup[769 + cr];
};

zhash_t Zobrist::ep_square(square sq)
{
    assert(is_valid(sq));
    return hash_value_lookup[HASH_VALUE_LOOKUP_SIZE - file_num(sq)];
};