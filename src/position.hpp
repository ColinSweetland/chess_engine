#ifndef GAMESTATE_INCL
#define GAMESTATE_INCL

#include "bitboard.hpp"
#include "types.hpp"

#include <array>
#include <iostream>
#include <vector>

class Position
{
  private:
    // two for each color, 6 for pieces, 1 for enpassante sq
    std::array<bitboard, 9> pos_bbs{0};

    // all moves that can be unmade
    std::vector<rev_move_data> unmake_stack{};

    unsigned int castle_r{0};

    unsigned int rev_moves{0};
    unsigned int full_moves{0};

    COLOR stm{WHITE};

  public:
    // default to the starting position
    Position(str fenstr = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    inline const COLOR& side_to_move() const { return stm; }

    inline const unsigned int& full_move_count() const { return full_moves; }
    inline const unsigned int& rev_move_count() const { return rev_moves; }

    inline const unsigned int& castle_rights() const { return castle_r; }

    PIECE piece_at_sq(square sq) const;

    bool sq_attacked(square sq, COLOR attacking_color) const;
    bool is_check(COLOR c) const;

    // returns all pseudolegal moves, also sets move_count to number of moves generated
    move_list pseudo_legal_moves() const;

    void make_move(const ChessMove c);
    bool try_make_move(const ChessMove c);

    void unmake_last();

    inline bitboard        pieces() const { return pos_bbs[WHITE] | pos_bbs[BLACK]; }
    inline const bitboard& pieces(int color_or_piece) const { return pos_bbs[color_or_piece]; }
    inline bitboard        pieces(COLOR c, PIECE p) const { return pos_bbs[c] & pos_bbs[p]; }

    str FEN() const;

    inline void print_bitboard(bitboard bb) const { print_bb(pos_bbs[bb]); }

    void dbg_print() const;

    friend std::ostream& operator<<(std::ostream& out, const Position& p);
};

#endif // GAMESTATE_INCL
