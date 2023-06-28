// need for PEXT/PDEP
#include <array>
#include <cstddef>
#include <x86intrin.h>

#include "bitboard.hpp"
#include "chessmove.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"

// when we shift east/west, wrapping can happen. To avoid this, we have to mask a row
static constexpr bitboard e_mask = ~BB_FILE_A;
static constexpr bitboard w_mask = ~BB_FILE_H;

// ---------------- MOVES ---------------------------

std::ostream& operator<<(std::ostream& out, const ChessMove& m)
{
    square orig = m.get_origin_sq();
    square dest = m.get_dest_sq();

    out << FILE_CHAR_FROM_SQ(orig) << RANK_CHAR_FROM_SQ(orig);

    out << FILE_CHAR_FROM_SQ(dest) << RANK_CHAR_FROM_SQ(dest);

    if (m.is_promo())
        out << piece_to_char.at(m.get_promo_piece());

    return out;
}

// returns amount of (pseudo legal) moves generated, and populates ml with them (ml should be at least len ~250)
move_list Position::pseudo_legal_moves() const
{
    move_list pl_moves{};

    // info: https://chess.stackexchange.com/questions/23135/what-is-the-average-number-of-legal-moves-per-turn
    // Average legal moves is about 30.
    // Average legal moves at peak is about 40
    // since we are generating pseudolegal, we need some extra space also. 50 sounds good
    // so in the vast majority of normal positions, we will need 0 resizes
    // in extreme positions we might need 1
    // in most extreme we would need 2
    pl_moves.reserve(50);

    const bitboard occ   = pieces();
    const bitboard pawns = pieces(stm, PAWN);

    const bitboard pawn_capturable  = pieces(static_cast<COLOR>(!stm)) | pieces(EN_PASSANTE);
    const bitboard moveable_squares = ~pieces(stm);

    const DIR pawn_push_dir   = PAWN_PUSH_DIR(stm);
    const int pawn_promo_rank = PAWN_PROMO_RANK(stm);

    bitboard p_single = bb_pawn_single_moves(pawns, occ, stm);
    bitboard p_double = bb_pawn_double_moves(p_single, occ, stm);
    bitboard p_att_e  = bb_pawn_attacks_e(pawns, pawn_capturable, stm);
    bitboard p_att_w  = bb_pawn_attacks_w(pawns, pawn_capturable, stm);

    // --- PAWNS ---

    // single pawn pushes
    while (p_single != BB_ZERO)
    {
        // square of a move
        square dest_sq = BB_LSB(p_single);
        BB_UNSET_LSB(p_single);

        square orig_sq = dest_sq - pawn_push_dir;

        if (RANK_FROM_SQ(dest_sq) == pawn_promo_rank)
        {
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::flags::KNIGHT_PROMO});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::flags::BISHOP_PROMO});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::flags::ROOK_PROMO});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::flags::QUEEN_PROMO});
        }
        else
        {
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::flags::NO_FLAGS});
        }
    }

    // double pawn pushes
    while (p_double != BB_ZERO)
    {
        // square of a move
        square dest_sq = BB_LSB(p_double);
        BB_UNSET_LSB(p_double);

        square orig_sq = dest_sq - (pawn_push_dir * 2);

        pl_moves.push_back({orig_sq, dest_sq, ChessMove::flags::DOUBLE_PUSH});
    }

    // pawn attacks east
    while (p_att_e != BB_ZERO)
    {
        // square of a move
        square dest_sq = BB_LSB(p_att_e);
        BB_UNSET_LSB(p_att_e);

        square orig_sq = dest_sq - (pawn_push_dir + EAST);

        PIECE captured = piece_at_sq(dest_sq);

        captured = captured == NO_PIECE ? EN_PASSANTE : captured;

        if (RANK_FROM_SQ(dest_sq) == pawn_promo_rank)
        {
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, KNIGHT)});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, BISHOP)});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, ROOK)});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, QUEEN)});
        }
        else
        {
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, NO_PIECE)});
        }
    }

    // pawn attacks west
    while (p_att_w != BB_ZERO)
    {
        // square of a move
        square dest_sq = BB_LSB(p_att_w);
        BB_UNSET_LSB(p_att_w);

        square orig_sq = dest_sq - (pawn_push_dir + WEST);

        PIECE captured = piece_at_sq(dest_sq);

        captured = captured == NO_PIECE ? EN_PASSANTE : captured;

        if (RANK_FROM_SQ(dest_sq) == pawn_promo_rank)
        {
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, KNIGHT)});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, BISHOP)});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, ROOK)});
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, QUEEN)});
        }
        else
        {
            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, NO_PIECE)});
        }
    }

    // --- KNIGHTS ---

    bitboard kn       = pieces(stm, KNIGHT);
    bitboard kn_moves = BB_ZERO;

    while (kn != BB_ZERO)
    {
        square orig_sq = BB_LSB(kn);
        BB_UNSET_LSB(kn);

        kn_moves = bb_knight_moves(orig_sq) & moveable_squares;

        while (kn_moves != BB_ZERO)
        {
            square dest_sq = BB_LSB(kn_moves);
            BB_UNSET_LSB(kn_moves);

            PIECE captured = piece_at_sq(dest_sq);

            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, NO_PIECE)});
        }
    }

    // --- BISHOPS ---

    bitboard bsh       = pieces(stm, BISHOP);
    bitboard bsh_moves = BB_ZERO;

    while (bsh != BB_ZERO)
    {
        square orig_sq = BB_LSB(bsh);
        BB_UNSET_LSB(bsh);

        bsh_moves = bb_bishop_moves(orig_sq, occ) & moveable_squares;

        while (bsh_moves != BB_ZERO)
        {
            square dest_sq = BB_LSB(bsh_moves);
            BB_UNSET_LSB(bsh_moves);

            PIECE captured = piece_at_sq(dest_sq);

            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, NO_PIECE)});
        }
    }

    // --- ROOKS ---

    bitboard rk       = pieces(stm, ROOK);
    bitboard rk_moves = BB_ZERO;

    while (rk != BB_ZERO)
    {
        square orig_sq = BB_LSB(rk);
        BB_UNSET_LSB(rk);

        rk_moves = bb_rook_moves(orig_sq, occ) & moveable_squares;

        while (rk_moves != BB_ZERO)
        {
            square dest_sq = BB_LSB(rk_moves);
            BB_UNSET_LSB(rk_moves);

            PIECE captured = piece_at_sq(dest_sq);

            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, NO_PIECE)});
        }
    }

    // --- QUEENS ---

    bitboard qn = pieces(stm, QUEEN);

    bitboard qn_moves = BB_ZERO;

    while (qn != BB_ZERO)
    {
        square orig_sq = BB_LSB(qn);
        BB_UNSET_LSB(qn);

        qn_moves = bb_queen_moves(orig_sq, occ) & moveable_squares;

        while (qn_moves != BB_ZERO)
        {
            square dest_sq = BB_LSB(qn_moves);
            BB_UNSET_LSB(qn_moves);

            PIECE captured = piece_at_sq(dest_sq);

            pl_moves.push_back({orig_sq, dest_sq, ChessMove::makeflag(captured, NO_PIECE)});
        }
    }

    // --- KING ---

    bitboard kng = pieces(stm, KING);

    // there can only be one king
    square kng_sq = BB_LSB(kng);

    bitboard kng_moves = bb_king_moves(kng_sq) & moveable_squares;

    while (kng_moves != BB_ZERO)
    {
        square dest_sq = BB_LSB(kng_moves);
        BB_UNSET_LSB(kng_moves);

        PIECE captured = piece_at_sq(dest_sq);

        pl_moves.push_back({kng_sq, dest_sq, ChessMove::makeflag(captured, NO_PIECE)});
    }

    // --- CASTLING ---

    // this will give us the castle rights of the moving side only
    // so even if we are black, we still use "white" here in this section
    char moving_cr = ((castle_r) >> (stm * 2)) & WCR;

    bool check = is_check(stm);
    // kingside
    if (!check && (moving_cr & WKS))
    {
        // squares between rook and king must be free
        // aka the kingside rook can attack our king
        int kingside_rooksq = stm == BLACK ? 63 : 7;

        bool spaces_free = (bb_rook_moves(kingside_rooksq, occ) & kng) > 0;

        // the spaces the king moves through also can't be attacked
        bool attacked = sq_attacked(kng_sq + EAST, static_cast<COLOR>(!stm))
                     || sq_attacked(kng_sq + (EAST * 2), static_cast<COLOR>(!stm));

        // if these conditions met, we can castle kingside
        if (spaces_free & !attacked)
        {
            square dest_sq = kng_sq + (EAST * 2);

            pl_moves.push_back({kng_sq, dest_sq, ChessMove::flags::KINGSIDE_CASTLE});
        }
    }

    // Queenside, see comments above
    if (!check && (moving_cr & WQS))
    {
        int queenside_rooksq = stm == BLACK ? 56 : 0;

        bool spaces_free = (bb_rook_moves(queenside_rooksq, occ) & kng) > 0;

        bool attacked = sq_attacked(kng_sq + WEST, static_cast<COLOR>(!stm))
                     || sq_attacked(kng_sq + (WEST * 2), static_cast<COLOR>(!stm));

        if (spaces_free & !attacked)
        {
            square dest_sq = kng_sq + (WEST * 2);

            pl_moves.push_back({kng_sq, dest_sq, ChessMove::flags::QUEENSIDE_CASTLE});
        }
    }

    pl_moves.shrink_to_fit(); // we are done adding elements
    return pl_moves;
}

// Adapted from chessprogramming wiki
// used for generating bishop and rook tables
static bitboard dumb7fill(square origin_sq, bitboard blockers, DIR* dirs)
{
    bitboard moves_bb = BB_ZERO;
    bitboard dirmask  = ~BB_ZERO;

    // diri indicates the direction we are filling, and mask to apply (above two tables)
    for (int dir_idx = 0; dir_idx < 4; dir_idx++)
    {
        DIR current_dir = dirs[dir_idx];

        switch (current_dir)
        {
        case NORTHEAST:
        case EAST:
        case SOUTHEAST:
            dirmask = e_mask;
            break;

        case NORTHWEST:
        case WEST:
        case SOUTHWEST:
            dirmask = w_mask;
            break;

        case NORTH:
        case SOUTH:
            break;
        }

        // empty squares = 1, occupied = 0
        bitboard prop = ~blockers & dirmask;

        // the square the rook is on
        bitboard r = BB_SQ(origin_sq);

        // we fill this with valid moves until blocker
        bitboard flood = BB_ZERO;

        // push rook along dir, until a blocker makes it dissapear
        while (r)
        {
            flood |= r;
            r = GEN_SHIFT(r, current_dir) & prop;
        }

        // shift one more time to get attacks/edges
        flood = GEN_SHIFT(flood, current_dir) & dirmask;

        moves_bb |= flood;
    }

    return moves_bb;
}

// ------------------ BISHOPS -----------------------

// lookup moves [idxlookup[sq] + PEXT(blockers)]
// we can make this more memory eff later (many elements are copies of eachother)
// size: ~41kb (8 bytes (bitboard) * 5185 entries)
std::array<std::vector<bitboard>, 64> BISHOP_MOVE_LOOKUP = {{}};

// squares set might contain pieces that block bishop moves
static constexpr std::array<const bitboard, 64> BISHOP_BLOCKER_MASK = {
    0x0040201008040200, 0x0000402010080400, 0x0000004020100a00, 0x0000000040221400, 0x0000000002442800,
    0x0000000204085000, 0x0000020408102000, 0x0002040810204000, 0x0020100804020000, 0x0040201008040000,
    0x00004020100a0000, 0x0000004022140000, 0x0000000244280000, 0x0000020408500000, 0x0002040810200000,
    0x0004081020400000, 0x0010080402000200, 0x0020100804000400, 0x004020100a000a00, 0x0000402214001400,
    0x0000024428002800, 0x0002040850005000, 0x0004081020002000, 0x0008102040004000, 0x0008040200020400,
    0x0010080400040800, 0x0020100a000a1000, 0x0040221400142200, 0x0002442800284400, 0x0004085000500800,
    0x0008102000201000, 0x0010204000402000, 0x0004020002040800, 0x0008040004081000, 0x00100a000a102000,
    0x0022140014224000, 0x0044280028440200, 0x0008500050080400, 0x0010200020100800, 0x0020400040201000,
    0x0002000204081000, 0x0004000408102000, 0x000a000a10204000, 0x0014001422400000, 0x0028002844020000,
    0x0050005008040200, 0x0020002010080400, 0x0040004020100800, 0x0000020408102000, 0x0000040810204000,
    0x00000a1020400000, 0x0000142240000000, 0x0000284402000000, 0x0000500804020000, 0x0000201008040200,
    0x0000402010080400, 0x0002040810204000, 0x0004081020400000, 0x000a102040000000, 0x0014224000000000,
    0x0028440200000000, 0x0050080402000000, 0x0020100804020000, 0x0040201008040200};

const bitboard& bb_bishop_moves(square sq, const bitboard& blockers)
{
    bitboard blocker_key = BB_PEXT(blockers, BISHOP_BLOCKER_MASK[sq]);
    return BISHOP_MOVE_LOOKUP.at(sq).at(blocker_key);
}

void init_bishop_table(void)
{
    DIR dirs[4] = {NORTHEAST, SOUTHEAST, NORTHWEST, SOUTHWEST};
    // iterate over each square
    for (square curr_sq = 0; curr_sq < 64; curr_sq++)
    {
        // blockers can appear in any bit set here
        bitboard potential_blocker_mask = BISHOP_BLOCKER_MASK[curr_sq];

        // we can have as many blockers as bits set
        int max_blockers = BB_POPCNT(potential_blocker_mask);

        // iterate through all combo of blockers in all relevant positions
        for (bitboard blockers = BB_ZERO; blockers < BB_SQ(max_blockers); blockers++)
        {
            // this particular combo
            bitboard blocker_set = BB_PDEP(blockers, potential_blocker_mask);

            bitboard moves = dumb7fill(curr_sq, blocker_set, dirs);

            // const cast because we want to be able to write to it now,
            // but never again
            BISHOP_MOVE_LOOKUP.at(curr_sq).push_back(moves);
        }

        BISHOP_MOVE_LOOKUP.at(curr_sq).shrink_to_fit();
    }
}

// ------------------ ROOKS -----------------------

// lookup moves [idxlookup[sq] + PEXT(blockers)]
// we can make this more memory eff later (many elements are copies of eachother)
// size: ~800kb (8 bytes (bitboard) * 102337 entries)
std::array<std::vector<bitboard>, 64> ROOK_MOVE_LOOKUP = {{}};

// squares set might contain pieces that block rook moves
constexpr std::array<const bitboard, 64> ROOK_BLOCKER_MASK = {
    0x000101010101017e, 0x000202020202027c, 0x000404040404047a, 0x0008080808080876, 0x001010101010106e,
    0x002020202020205e, 0x004040404040403e, 0x008080808080807e, 0x0001010101017e00, 0x0002020202027c00,
    0x0004040404047a00, 0x0008080808087600, 0x0010101010106e00, 0x0020202020205e00, 0x0040404040403e00,
    0x0080808080807e00, 0x00010101017e0100, 0x00020202027c0200, 0x00040404047a0400, 0x0008080808760800,
    0x00101010106e1000, 0x00202020205e2000, 0x00404040403e4000, 0x00808080807e8000, 0x000101017e010100,
    0x000202027c020200, 0x000404047a040400, 0x0008080876080800, 0x001010106e101000, 0x002020205e202000,
    0x004040403e404000, 0x008080807e808000, 0x0001017e01010100, 0x0002027c02020200, 0x0004047a04040400,
    0x0008087608080800, 0x0010106e10101000, 0x0020205e20202000, 0x0040403e40404000, 0x0080807e80808000,
    0x00017e0101010100, 0x00027c0202020200, 0x00047a0404040400, 0x0008760808080800, 0x00106e1010101000,
    0x00205e2020202000, 0x00403e4040404000, 0x00807e8080808000, 0x007e010101010100, 0x007c020202020200,
    0x007a040404040400, 0x0076080808080800, 0x006e101010101000, 0x005e202020202000, 0x003e404040404000,
    0x007e808080808000, 0x7e01010101010100, 0x7c02020202020200, 0x7a04040404040400, 0x7608080808080800,
    0x6e10101010101000, 0x5e20202020202000, 0x3e40404040404000, 0x7e80808080808000};

// Where you can begin to lookup the move bb for a certain square
// for example, in a1 we have 4095 (2^12 - 1) possible blocker combos,
// because there are 12 squares where blockers can be
// so for a2 we start at index 4095 (4096th element)
// max idx = 102336
// constexpr std::array<const uint32_t, 64> ROOK_MOVE_START_IDX = {
//     0,     4095,  6142,  8189,  10236, 12283, 14330, 16377, 20472, 22519, 23542, 24565, 25588, 26611, 27634, 28657,
//     30704, 32751, 33774, 34797, 35820, 36843, 37866, 38889, 40936, 42983, 44006, 45029, 46052, 47075, 48098, 49121,
//     51168, 53215, 54238, 55261, 56284, 57307, 58330, 59353, 61400, 63447, 64470, 65493, 66516, 67539, 68562, 69585,
//     71632, 73679, 74702, 75725, 76748, 77771, 78794, 79817, 81864, 85959, 88006, 90053, 92100, 94147, 96194, 98241};

const bitboard& bb_rook_moves(square sq, const bitboard& blockers)
{
    bitboard blocker_key = BB_PEXT(blockers, ROOK_BLOCKER_MASK[sq]);
    return ROOK_MOVE_LOOKUP.at(sq).at(blocker_key);
}

void init_rook_table(void)
{
    DIR dirs[4] = {NORTH, SOUTH, EAST, WEST};
    // iterate over each square
    for (square curr_sq = 0; curr_sq < 64; curr_sq++)
    {
        // blockers can appear in any bit set here
        bitboard potential_blocker_mask = ROOK_BLOCKER_MASK[curr_sq];

        // we can have as many blockers as bits set
        int max_blockers = BB_POPCNT(potential_blocker_mask);

        // iterate through all combo of blockers in all relevant positions
        for (bitboard blockers = BB_ZERO; blockers < BB_SQ(max_blockers); blockers++)
        {
            // this particular combo
            bitboard blocker_set = BB_PDEP(blockers, potential_blocker_mask);

            bitboard moves = dumb7fill(curr_sq, blocker_set, dirs);

            ROOK_MOVE_LOOKUP.at(curr_sq).push_back(moves);
        }

        ROOK_MOVE_LOOKUP.at(curr_sq).shrink_to_fit();
    }
}

// ------------------QUEENS----------------------

bitboard bb_queen_moves(square sq, const bitboard& blockers)
{
    return bb_rook_moves(sq, blockers) | bb_bishop_moves(sq, blockers);
}

// ------------------KINGS-----------------------

static const bitboard king_lookup[64] = {
    0x0000000000000302ULL, 0x0000000000000705ULL, 0x0000000000000e0aULL, 0x0000000000001c14ULL, 0x0000000000003828ULL,
    0x0000000000007050ULL, 0x000000000000e0a0ULL, 0x000000000000c040ULL, 0x0000000000030203ULL, 0x0000000000070507ULL,
    0x00000000000e0a0eULL, 0x00000000001c141cULL, 0x0000000000382838ULL, 0x0000000000705070ULL, 0x0000000000e0a0e0ULL,
    0x0000000000c040c0ULL, 0x0000000003020300ULL, 0x0000000007050700ULL, 0x000000000e0a0e00ULL, 0x000000001c141c00ULL,
    0x0000000038283800ULL, 0x0000000070507000ULL, 0x00000000e0a0e000ULL, 0x00000000c040c000ULL, 0x0000000302030000ULL,
    0x0000000705070000ULL, 0x0000000e0a0e0000ULL, 0x0000001c141c0000ULL, 0x0000003828380000ULL, 0x0000007050700000ULL,
    0x000000e0a0e00000ULL, 0x000000c040c00000ULL, 0x0000030203000000ULL, 0x0000070507000000ULL, 0x00000e0a0e000000ULL,
    0x00001c141c000000ULL, 0x0000382838000000ULL, 0x0000705070000000ULL, 0x0000e0a0e0000000ULL, 0x0000c040c0000000ULL,
    0x0003020300000000ULL, 0x0007050700000000ULL, 0x000e0a0e00000000ULL, 0x001c141c00000000ULL, 0x0038283800000000ULL,
    0x0070507000000000ULL, 0x00e0a0e000000000ULL, 0x00c040c000000000ULL, 0x0302030000000000ULL, 0x0705070000000000ULL,
    0x0e0a0e0000000000ULL, 0x1c141c0000000000ULL, 0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL,
    0xc040c00000000000ULL, 0x0203000000000000ULL, 0x0507000000000000ULL, 0x0a0e000000000000ULL, 0x141c000000000000ULL,
    0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL};

// The thrill is gone
const bitboard& bb_king_moves(square sq) { return king_lookup[sq]; }

//-------------------KNIGHTS---------------------
static const bitboard knight_lookup[64] = {
    0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000A1100ULL, 0x0000000000142200ULL, 0x0000000000284400ULL,
    0x0000000000508800ULL, 0x0000000000A01000ULL, 0x0000000000402000ULL, 0x0000000002040004ULL, 0x0000000005080008ULL,
    0x000000000A110011ULL, 0x0000000014220022ULL, 0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000A0100010ULL,
    0x0000000040200020ULL, 0x0000000204000402ULL, 0x0000000508000805ULL, 0x0000000A1100110AULL, 0x0000001422002214ULL,
    0x0000002844004428ULL, 0x0000005088008850ULL, 0x000000A0100010A0ULL, 0x0000004020002040ULL, 0x0000020400040200ULL,
    0x0000050800080500ULL, 0x00000A1100110A00ULL, 0x0000142200221400ULL, 0x0000284400442800ULL, 0x0000508800885000ULL,
    0x0000A0100010A000ULL, 0x0000402000204000ULL, 0x0002040004020000ULL, 0x0005080008050000ULL, 0x000A1100110A0000ULL,
    0x0014220022140000ULL, 0x0028440044280000ULL, 0x0050880088500000ULL, 0x00A0100010A00000ULL, 0x0040200020400000ULL,
    0x0204000402000000ULL, 0x0508000805000000ULL, 0x0A1100110A000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL,
    0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL, 0x0400040200000000ULL, 0x0800080500000000ULL,
    0x1100110A00000000ULL, 0x2200221400000000ULL, 0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010A000000000ULL,
    0x2000204000000000ULL, 0x0004020000000000ULL, 0x0008050000000000ULL, 0x00110A0000000000ULL, 0x0022140000000000ULL,
    0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010A00000000000ULL, 0x0020400000000000ULL};

const bitboard& bb_knight_moves(square sq) { return knight_lookup[sq]; }

// ------------------PAWNS-----------------------
// 1. attacks
bitboard bb_pawn_attacks_w(const bitboard& pawns, const bitboard& attackable, COLOR moving)
{
    DIR attack_dir = static_cast<DIR>(PAWN_PUSH_DIR(moving) + WEST);

    return GEN_SHIFT(pawns, attack_dir) & attackable & w_mask;
}

bitboard bb_pawn_attacks_e(const bitboard& pawns, const bitboard& attackable, COLOR moving)
{
    DIR attack_dir = static_cast<DIR>(PAWN_PUSH_DIR(moving) + EAST);

    return GEN_SHIFT(pawns, attack_dir) & attackable & e_mask;
}

// 2. Moves
bitboard bb_pawn_single_moves(const bitboard& pawns, const bitboard& blockers, COLOR side_to_move)
{
    DIR move_dir = PAWN_PUSH_DIR(side_to_move);

    return GEN_SHIFT(pawns, move_dir) & ~blockers;
}

bitboard bb_pawn_double_moves(const bitboard& single_moves, const bitboard& blockers, COLOR side_to_move)
{
    DIR move_dir = PAWN_PUSH_DIR(side_to_move);

    bitboard double_move_rank = side_to_move == BLACK ? BB_RANK_5 : BB_RANK_4;

    return GEN_SHIFT(single_moves, move_dir) & ~blockers & double_move_rank;
}
