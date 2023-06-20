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

    unsigned int castle_r;

    unsigned int rev_moves;
    unsigned int full_moves;

    COLOR stm;

  public:
    // default to the starting position
    Position(const char* fenstr = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    inline const COLOR& side_to_move() const { return stm; }

    inline const unsigned int& full_move_count() const { return full_moves; }
    inline const unsigned int& rev_move_count() const { return rev_moves; }

    inline const unsigned int& castle_rights() const { return castle_r; }

    PIECE piece_at_sq(int sq) const;

    bool sq_attacked(int sq, COLOR attacking_color) const;

    // returns all pseudolegal moves, also sets move_count to number of moves generated
    int pseudo_legal_moves(std::array<ChessMove, MAX_GENERATABLE_MOVES>& pl_moves) const;

    void make_move(const ChessMove c);
    void unmake_last();

    inline bitboard        pieces() const { return pos_bbs[WHITE] | pos_bbs[BLACK]; }
    inline const bitboard& pieces(int color_or_piece) const { return pos_bbs[color_or_piece]; }
    inline bitboard        pieces(COLOR c, PIECE p) const { return pos_bbs[c] & pos_bbs[p]; }

    str FEN() const;

    inline void print_bitboard(int bb) const { print_bb(pos_bbs[bb]); }

    void dbg_print() const;

    friend std::ostream& operator<<(std::ostream& out, const Position& p);
};

#endif // GAMESTATE_INCL
