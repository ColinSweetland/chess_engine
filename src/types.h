#ifndef TYPES_INCL
#define TYPES_INCL

#include <cinttypes>
#include <string>
#include <unordered_map>

/* This file will define the basic types used in the engine */

// alias for stdlib string
using str = std::string;

using bitboard = std::uint64_t;

// 0-63 square on bitboard, from a1 to h8. -1 used for no square
using square = int;

enum COLOR
{
    WHITE,
    BLACK
};

// direction on the board, from white's point of view
enum DIR
{
    NORTH = 8,
    EAST  = 1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTHWEST = NORTH + WEST,
    NORTHEAST = NORTH + EAST,
    SOUTHEAST = SOUTH + EAST,
    SOUTHWEST = SOUTH + WEST,
};

inline DIR PAWN_PUSH_DIR(COLOR c) { return c ? SOUTH : NORTH; }

enum PIECE
{
    NO_PIECE = -1,
    PAWN     = 2, // start at two because index 0 & 1 represent color
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    EN_PASSANTE
};

const std::unordered_map<char, PIECE> char_to_piece = {{'p', PAWN}, {'n', KNIGHT}, {'b', BISHOP},
                                                       {'r', ROOK}, {'q', QUEEN},  {'k', KING}};

const std::unordered_map<int, char> piece_to_char = {{WHITE, 'w'},       {BLACK, 'b'},   {PAWN, 'p'},  {KNIGHT, 'n'},
                                                     {BISHOP, 'b'},      {ROOK, 'r'},    {QUEEN, 'q'}, {KING, 'k'},
                                                     {EN_PASSANTE, 'e'}, {NO_PIECE, '.'}};

const std::unordered_map<int, str> piece_to_str = {
    {WHITE, "WHITE"}, {BLACK, "BLACK"}, {PAWN, "PAWN"}, {KNIGHT, "KNIGHT"},           {BISHOP, "BISHOP"},
    {ROOK, "ROOK"},   {QUEEN, "QUEEN"}, {KING, "KING"}, {EN_PASSANTE, "EN_PASSANTE"}, {NO_PIECE, "NO_PIECE"}};

enum CASTLE_RIGHT
{
    NO_RIGHTS = 0,
    WQS       = 1,
    WKS       = 2,
    BQS       = 4,
    BKS       = 8,

    WCR = WQS | WKS,
    BCR = BQS | BKS
};

// Move = 16 bits
// 0-5 from sq
// 6-11 to sq
// 12-15 flags

// Credit: Chessprogamming wiki https://www.chessprogramming.org/Encoding_Moves#From-To_Based
// Also: Stockfish (has similar representation)
class ChessMove
{
  private:
    uint16_t m_move;

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

    // inline void operator=(ChessMove a) { m_move = a.m_move; }

    inline square       get_dest_sq() const { return m_move & 0x3f; }
    inline square       get_origin_sq() const { return (m_move >> 6) & 0x3f; }
    inline unsigned int get_flags() const { return (m_move >> 12) & 0x0f; }

    inline PIECE get_promo_piece() const
    {
        return static_cast<PIECE>(is_promo() ? (get_flags() - (is_capture() ? 9 : 5)) : NO_PIECE);
    }

    void set_to(square to)
    {
        m_move &= ~0x3f;
        m_move |= to & 0x3f;
    }
    void set_from(unsigned int from)
    {
        m_move &= ~0xfc0;
        m_move |= (from & 0x3f) << 6;
    }

    bool is_promo() const { return get_flags() & 8; }
    bool is_capture() const { return get_flags() & 4; }
    bool is_castle() const
    {
        int f = get_flags();
        return f == flags::KINGSIDE_CASTLE || f == QUEENSIDE_CASTLE;
    }
    bool is_double_push() const { return get_flags() == DOUBLE_PUSH; }
    bool is_en_passante() const { return get_flags() == EP_CAP; }

    bool operator==(ChessMove a) const { return m_move == a.m_move; }
    bool operator!=(ChessMove a) const { return m_move != a.m_move; }

    // helper - returns flag with capture/promo
    static constexpr ChessMove::flags makeflag(PIECE cap, PIECE pro)
    {
        int f = 0;
        if (pro == NO_PIECE)
            switch (cap)
            {
            case NO_PIECE:
                f = NO_FLAGS;
                break;
            case EN_PASSANTE:
                f = EP_CAP;
                break;
            default:
                f = CAPTURE;
                break;
            }
        else
        {
            f = cap == NO_PIECE ? KNIGHT_PROMO : KNIGHT_PROMO_CAP;
            f += (pro - KNIGHT);
        }

        return static_cast<ChessMove::flags>(f);
    }
};

// contains all data needed to unmake a move
struct rev_move_data
{
    ChessMove    move;
    unsigned int rev_move_clock;
    unsigned int castle_r;
    PIECE        captured_pieceS;
    bitboard     enp_bb;
};

const int MAX_GENERATABLE_MOVES = {256};

using move_list = std::array<ChessMove, MAX_GENERATABLE_MOVES>;

#endif // TYPES_INCL