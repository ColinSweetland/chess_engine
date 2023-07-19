#ifndef CHESS_MOVE_INCL
#define CHESS_MOVE_INCL

#include <stdint.h>

#include "bitboard.hpp"
#include "types.hpp"

// Move = 16 bits
// 0-5 from sq
// 6-11 to sq
// 12-15 flags

// Credit: Chessprogamming wiki https://www.chessprogramming.org/Encoding_Moves#From-To_Based
// Also: Stockfish (has similar representation)
class ChessMove
{
  private:
    std::uint16_t m_move;

  public:
    enum flags
    {
        NO_FLAGS    = 0,
        DOUBLE_PUSH = 1,

        // Castling has bit 1 set
        KINGSIDE_CASTLE  = 2,
        QUEENSIDE_CASTLE = 3,

        // Captures have bit 2 set
        // Q: is it even necessary to indicate captures, since we can just check piece set at dest sq?
        CAPTURE          = 4,
        EP_CAP           = 5,
        UNUSED_CAP_FLAG1 = 6,
        UNUSED_CAP_FLAG2 = 7,

        // promos have bit 3 set, maybe we don't need all promos represented
        // (promoting to anything but knight or queen is always suboptimal)
        KNIGHT_PROMO     = 8,
        BISHOP_PROMO     = 9,
        ROOK_PROMO       = 10,
        QUEEN_PROMO      = 11,
        KNIGHT_PROMO_CAP = 12,
        BISHOP_PROMO_CAP = 13,
        ROOK_PROMO_CAP   = 14,
        QUEEN_PROMO_CAP  = 15,
    };

    inline ChessMove(square orig = 0, square dest = 0, int flags = 0)
    {
        m_move = ((flags & 0xf) << 12) | ((orig & 0x3f) << 6) | (dest & 0x3f);
    }

    inline square       get_dest_sq() const { return m_move & 0x3f; }
    inline square       get_origin_sq() const { return (m_move >> 6) & 0x3f; }
    inline unsigned int get_flags() const { return (m_move >> 12) & 0x0f; }

    inline PIECE get_promo_piece() const
    {
        return static_cast<PIECE>(is_promo() ? (get_flags() - (is_capture() ? 9 : 5)) : NO_PIECE);
    }

    inline bool is_promo() const { return get_flags() & 8; }
    inline bool is_capture() const { return get_flags() & 4; }

    inline bool is_castle() const
    {
        int f = get_flags();
        return f == flags::KINGSIDE_CASTLE || f == QUEENSIDE_CASTLE;
    }

    inline bool is_double_push() const { return get_flags() == DOUBLE_PUSH; }
    inline bool is_en_passante() const { return get_flags() == EP_CAP; }

    inline str to_str() const
    {
        str temp = SQ_TO_STR(get_origin_sq()) + SQ_TO_STR(get_dest_sq());
        return is_promo() ? temp + piece_to_char.at(get_promo_piece()) : temp;
    }

    // helper - returns flag with capture/promo
    static constexpr ChessMove::flags makeflag(PIECE cap, PIECE pro)
    {
        int f = 0;
        if (pro == NO_PIECE)
            switch (cap)
            {
            case NO_PIECE:
                f = ChessMove::NO_FLAGS;
                break;
            case EN_PASSANTE:
                f = ChessMove::EP_CAP;
                break;
            default:
                f = ChessMove::CAPTURE;
                break;
            }
        else
        {
            f = cap == NO_PIECE ? ChessMove::KNIGHT_PROMO : ChessMove::KNIGHT_PROMO_CAP;
            f += (pro - KNIGHT);
        }

        return static_cast<ChessMove::flags>(f);
    }
};

inline std::ostream& operator<<(std::ostream& out, const ChessMove& m) { return out << m.to_str(); }

// contains all data needed to unmake a move
struct rev_move_data
{
    ChessMove    move;
    unsigned int rev_move_clock;
    unsigned int castle_r;
    PIECE        captured_piece;
    bitboard     enp_bb;
};

using move_list = std::vector<ChessMove>;

using scored_move = std::pair<ChessMove, int32_t>;

#endif