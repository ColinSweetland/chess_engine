#ifndef GAMESTATE_INCL
#define GAMESTATE_INCL

#include "./types/bitboard.hpp"
#include "./types/pieces.hpp"
#include "chessmove.hpp"
#include "zobrist.hpp"

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
    square       prev_enp_sq{0};

    bitboard checkers_bb{0};

    // need a constructor to use emplace_back
    state_info(ChessMove cm, unsigned int rmc, unsigned int pcr, square pesq)
        : prev_move(cm), prev_rev_move_count(rmc), prev_castle_r(pcr), prev_enp_sq(pesq)
    {
    }
};

class Position
{
  public:
    enum CASTLE_RIGHT
    {
        CR_WQS = 1,
        CR_WKS = 2,
        CR_BQS = 4,
        CR_BKS = 8,

        CR_WKQS = CR_WKS | CR_WQS,
        CR_BKQS = CR_BKS | CR_BQS,

        CR_ANY = CR_WQS | CR_WKS | CR_BQS | CR_BKS
    };

  private:
    // bitboard for each piece, index with COLOR then PIECE
    bitboard m_piece_bbs[2][7];

    bitboard m_color_bbs[2];

    square m_enp_sq;

    // all historical state info
    std::vector<state_info> m_state_info_stack{};

    zhash_t m_curr_zhash;

    unsigned int m_castle_r;
    unsigned int m_rev_move_count;
    unsigned int m_full_moves;

    COLOR m_stm{WHITE};

    // helpers
    void remove_piece(COLOR c, PIECE p, square sq);
    void place_piece(COLOR c, PIECE p, square sq);
    void move_piece(COLOR c, PIECE p, square orig, square dest);
    void move_and_change_piece(COLOR c, PIECE orig_p, PIECE new_p, square orig_sq, square dest_sq);

    bool has_cr(CASTLE_RIGHT cr) const { return m_castle_r & cr; }
    void remove_cr(CASTLE_RIGHT cr) { m_castle_r &= ~cr; }
    void give_cr(CASTLE_RIGHT cr) { m_castle_r |= cr; }

    std::string castle_right_str() const;

    void update_checkers_bb();

  public:
    // default to the starting position
    Position(std::string fenstr = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    inline const COLOR& side_to_move() const { return m_stm; }

    // self explanatory but the rules of chess aren't
    // after this is true, the game can be a draw or win,
    // but it depends on if their are legal moves.
    // if the side to move has legal moves, draw, else checkmated
    inline bool has_been_50_reversible_full_moves() { return m_rev_move_count >= 100; }

    inline const unsigned int& full_move_count() const { return m_full_moves; }
    inline const unsigned int& rev_move_count() const { return m_rev_move_count; }
    inline square              en_passante_sq() const { return m_enp_sq; }

    PIECE piece_at_sq(square sq) const;
    COLOR color_at_sq(square sq) const;

    bool sq_attacked(square sq, COLOR attacking_color) const;

    // returns all pseudolegal moves, also sets move_count to number of moves generated
    move_list pseudo_legal_moves() const;

    move_list legal_moves();

    const bitboard& get_checkers_bb() const { return m_state_info_stack.back().checkers_bb; }

    bool is_check() const { return get_checkers_bb(); }

    const ChessMove& last_move() const { return m_state_info_stack.back().prev_move; }

    zhash_t zhash() const { return m_curr_zhash; }

    void make_move(const ChessMove c);
    bool try_make_move(const ChessMove c);

    void unmake_last();

    inline bitboard pieces() const { return m_color_bbs[WHITE] | m_color_bbs[BLACK]; }
    inline bitboard pieces(COLOR c) const { return m_color_bbs[c]; }
    inline bitboard pieces(PIECE p) const { return m_piece_bbs[WHITE][p] | m_piece_bbs[BLACK][p]; }
    inline bitboard pieces(COLOR c, PIECE p) const { return m_piece_bbs[c][p]; }

    std::string FEN() const;

    void dump_move_history() const;

    void dump_zhash() const;

    friend std::ostream& operator<<(std::ostream& out, const Position& p);
};

#endif // GAMESTATE_INCL
