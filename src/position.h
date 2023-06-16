#ifndef GAMESTATE_INCL
#define GAMESTATE_INCL

#include "bitboard.h"
#include "types.h"

#include <array>
#include <iostream>

class Position
{
  private:
    // two for each color, 6 for pieces, 1 for enpassante sq
    std::array<bitboard, 9> pos_bbs;

    int castle_r;

    unsigned int rev_moves;
    unsigned int full_moves;

    COLOR stm;

  public:
    // default to the starting position
    Position(const char* fenstr = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    inline const COLOR& side_to_move() const { return stm; }

    inline const unsigned int& full_move_count() const { return full_moves; }
    inline const unsigned int& rev_move_count() const { return rev_moves; }

    inline const int& castle_rights() const { return castle_r; }

    PIECE piece_at_sq(int sq) const;

    bool sq_attacked(int sq, COLOR attacking_color) const;

    // returns all pseudolegal moves, also sets move_count to number of moves generated
    std::array<chess_move, MAX_GENERATABLE_MOVES> pseudo_legal_moves(int& move_count) const;

    void make_move(chess_move c);
    void unmake_last();

    inline bitboard        pieces() const { return pos_bbs[WHITE] | pos_bbs[BLACK]; }
    inline const bitboard& pieces(COLOR c) const { return pos_bbs[c]; }
    inline const bitboard& pieces(PIECE p) const { return pos_bbs[p]; }
    inline bitboard        pieces(COLOR c, PIECE p) const { return pos_bbs[c] & pos_bbs[p]; }

    str FEN() const;

    inline void print_bitboard(int bb) const { print_bb(pos_bbs[bb]); }

    void dbg_print() const;

    friend std::ostream& operator<<(std::ostream& out, const Position& p);
};

/*
bool sq_attacked(const game_state* gs, int sq, COLOR attacking_color);
*/

#endif // GAMESTATE_INCL
