#ifndef CHESS_MOVE_INCL
#define CHESS_MOVE_INCL

#include "./types/bitboard.hpp"
#include "./types/pieces.hpp"
#include <vector>

class ChessMove
{
  private:
    // aftermove_piece is like promotion piece
    // except same as moved_piece for non promotions
    PIECE m_moved_piece;
    PIECE m_aftermove_piece;

    square m_orig_sq;
    square m_dest_sq;

    PIECE m_cap_piece;

  public:
    // default constructor (null move)
    ChessMove()
        : m_moved_piece(NO_PIECE), m_aftermove_piece(NO_PIECE), m_orig_sq(0), m_dest_sq(0), m_cap_piece(NO_PIECE)
    {
    }

    // regular constructor
    ChessMove(PIECE moved, square orig, square dest, PIECE cap = NO_PIECE)
        : m_moved_piece(moved), m_aftermove_piece(moved), m_orig_sq(orig), m_dest_sq(dest), m_cap_piece(cap)
    {
    }

    // promo constructor
    ChessMove(PIECE moved, PIECE promo, square orig, square dest, PIECE cap = NO_PIECE)
        : m_moved_piece(moved), m_aftermove_piece(promo), m_orig_sq(orig), m_dest_sq(dest), m_cap_piece(cap)
    {
    }

    // getters
    inline square get_orig() const { return m_orig_sq; }
    inline square get_dest() const { return m_dest_sq; }
    inline PIECE  get_moved_piece() const { return m_moved_piece; }
    inline PIECE  get_captured_piece() const { return m_cap_piece; }
    inline PIECE  get_after_move_piece() const { return m_aftermove_piece; }

    inline bool  is_promo() const { return m_moved_piece != m_aftermove_piece; }
    inline PIECE get_promo_piece() const { return is_promo() ? m_aftermove_piece : NO_PIECE; }

    inline bool is_capture() const { return m_cap_piece != NO_PIECE; }

    inline bool is_castle() const
    {
        return m_moved_piece == KING && file_num(m_orig_sq) == 5
            && (file_num(m_dest_sq) == 3 || file_num(m_dest_sq) == 7);
    }

    inline bool is_double_push() const
    {
        return m_moved_piece == PAWN && abs(rank_num(m_orig_sq) - rank_num(m_dest_sq)) == 2;
    }
    inline bool is_en_passante() const { return m_cap_piece == EN_PASSANTE; }

    // ie initialized, but not valid
    inline bool is_null() { return m_moved_piece == NO_PIECE; }

    inline std::string to_str() const
    {
        std::string temp = sq_str(m_orig_sq) + sq_str(m_dest_sq);
        return is_promo() ? temp + piece_to_char(m_aftermove_piece) : temp;
    }

    // prints out moveinfo, useful for debugging
    void dump_info() const;

    int score_for_ordering() const;
};

inline std::ostream& operator<<(std::ostream& out, const ChessMove& m) { return out << m.to_str(); }

using move_list = std::vector<ChessMove>;

void order_moves(move_list& ml);

#endif