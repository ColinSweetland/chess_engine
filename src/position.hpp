#ifndef GAMESTATE_INCL
#define GAMESTATE_INCL

#include "bitboard.hpp"
#include "chessmove.hpp"
#include "types.hpp"

#include <array>
#include <iostream>
#include <vector>

// the state info holds information about the current state of the game
// & contains all data needed to unmake a move
struct state_info
{
    // all info needed to unmake the last move
    ChessMove    prev_move{};
    unsigned int prev_rev_move_count{0};
    unsigned int prev_castle_r{0};
    bitboard     prev_enp_bb{0};

    bool is_check;
    // need a constructor to use emplace_back
    state_info(ChessMove cm, unsigned int rmc, unsigned int pcr, bitboard pebb)
        : prev_move(cm), prev_rev_move_count(rmc), prev_castle_r(pcr), prev_enp_bb(pebb)
    {
    }
};

class Position
{
  private:
    // two for each color, 6 for pieces, 1 for enpassante sq
    std::array<bitboard, 9> pos_bbs{0};

    // all historical state info
    std::vector<state_info> state_info_stack{};

    unsigned int castle_r{0};
    unsigned int rev_move_count{0};
    unsigned int full_moves{0};

    COLOR stm{WHITE};

    // helpers
    void remove_piece(COLOR c, PIECE p, square sq);
    void place_piece(COLOR c, PIECE p, square sq);
    void move_piece(COLOR c, PIECE p, square orig, square dest);
    void move_and_change_piece(COLOR c, PIECE orig_p, PIECE new_p, square orig_sq, square dest_sq);

    bool has_cr(CASTLE_RIGHT cr) const { return castle_r & cr; }
    void remove_cr(CASTLE_RIGHT cr) { castle_r &= ~cr; }
    void give_cr(CASTLE_RIGHT cr) { castle_r |= cr; }

    str castle_right_str() const;

  public:
    // default to the starting position
    Position(str fenstr = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    inline const COLOR& side_to_move() const { return stm; }

    // self explanatory but the rules of chess aren't
    // after this is true, the game can be a draw or win,
    // but it depends on if their are legal moves.
    // if the side to move has legal moves, draw, else checkmated
    inline bool has_been_50_reversible_full_moves() { return rev_move_count >= 100; }

    inline const unsigned int& full_move_count() const { return full_moves; }

    PIECE piece_at_sq(square sq) const;
    COLOR color_at_sq(square sq) const;

    bool sq_attacked(square sq, COLOR attacking_color) const;

    // returns all pseudolegal moves, also sets move_count to number of moves generated
    move_list pseudo_legal_moves() const;

    move_list legal_moves();

    inline bool      is_check() const { return state_info_stack.back().is_check; }
    const ChessMove& last_move() const { return state_info_stack.back().prev_move; }

    void make_move(const ChessMove c);
    bool try_make_move(const ChessMove c);

    void unmake_last();

    inline bitboard        pieces() const { return pos_bbs[WHITE] | pos_bbs[BLACK]; }
    inline const bitboard& pieces(int color_or_piece) const { return pos_bbs[color_or_piece]; }
    inline bitboard        pieces(COLOR c, PIECE p) const { return pos_bbs[c] & pos_bbs[p]; }

    str FEN() const;

    void dump_move_history() const;

    friend std::ostream& operator<<(std::ostream& out, const Position& p);
};

#endif // GAMESTATE_INCL
