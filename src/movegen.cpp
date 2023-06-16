// need for PEXT/PDEP
#include <array>
#include <x86intrin.h>

#include <iostream>

#include "bitboard.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

// when we shift east/west, wrapping can happen. To avoid this, we have to mask a row
static constexpr bitboard e_mask = ~BB_FILE_A;
static constexpr bitboard w_mask = ~BB_FILE_H;

// ---------------- MOVES ---------------------------

std::ostream& operator<<(std::ostream& out, const chess_move& move)
{

    // promo or castle move
    if (move.promo != NO_PIECE)
    {
        void print_move(const chess_move&);

        switch (move.movedp)
        {

        case PAWN: // pawn promotion
            out << "Pawn on " << FILE_CHAR_FROM_SQ(move.from_sq) << RANK_CHAR_FROM_SQ(move.from_sq);
            out << " moves to " << FILE_CHAR_FROM_SQ(move.to_sq) << RANK_CHAR_FROM_SQ(move.to_sq);
            out << " and promotes to " << piece_to_str.at(move.promo) << '\n';
            break;

        case KING:
            out << "King castles " << piece_to_str.at(move.promo) << "side\n";
            break;

        default:
            std::cerr << "Unexpected moving piece " << piece_to_str.at(move.movedp) << " when move promo field set to "
                      << piece_to_str.at(move.promo) << '\n';
            std::cerr << "File: " << __FILE__ << " line: " << __LINE__ << '\n';
            exit(1);
            break;
        }

        return out;
    }

    out << piece_to_str.at(move.movedp) << " on " << FILE_CHAR_FROM_SQ(move.from_sq) << RANK_CHAR_FROM_SQ(move.from_sq);

    if (move.captp == NO_PIECE)
    {
        out << " moves to ";
    }
    else
    {
        out << " Captures " << piece_to_str.at(move.captp) << " on ";
    }

    out << FILE_CHAR_FROM_SQ(move.to_sq) << RANK_CHAR_FROM_SQ(move.to_sq) << '\n';

    return out;
}

// returns amount of (pseudo legal) moves generated, and populates ml with them (ml should be at least len ~250)
std::array<chess_move, MAX_GENERATABLE_MOVES> Position::pseudo_legal_moves(int& move_count) const
{
    std::array<chess_move, MAX_GENERATABLE_MOVES> pl_moves;

    move_count = 0;

    chess_move m;

    const bitboard occ   = pieces();
    const bitboard pawns = pieces(stm, PAWN);

    // only used for pawns
    const bitboard enemy_pieces = pieces(static_cast<COLOR>(!stm)) | pieces(EN_PASSANTE);

    const bitboard moveable_squares = ~pieces(stm);

    bitboard p_single = bb_pawn_single_moves(pawns, occ, stm);
    bitboard p_double = bb_pawn_double_moves(p_single, occ, stm);
    bitboard p_att_e  = bb_pawn_attacks_e(pawns, enemy_pieces, stm);
    bitboard p_att_w  = bb_pawn_attacks_w(pawns, enemy_pieces, stm);

    DIR pawn_push_dir = PAWN_PUSH_DIR(stm);

    // --- PAWNS ---

    // single pawn pushes
    while (p_single != BB_ZERO)
    {
        // square of a move
        int sq = BB_LSB(p_single);
        BB_UNSET_LSB(p_single);

        pl_moves[move_count].movedp  = PAWN;
        pl_moves[move_count].captp   = NO_PIECE;
        pl_moves[move_count].to_sq   = sq;
        pl_moves[move_count].from_sq = sq - pawn_push_dir;
        pl_moves[move_count].promo   = NO_PIECE;

        move_count++;
    }

    // double pawn pushes
    while (p_double != BB_ZERO)
    {
        // square of a move
        int sq = BB_LSB(p_double);
        BB_UNSET_LSB(p_double);

        pl_moves[move_count].movedp  = PAWN;
        pl_moves[move_count].captp   = NO_PIECE;
        pl_moves[move_count].to_sq   = sq;
        pl_moves[move_count].from_sq = sq - pawn_push_dir * 2;
        pl_moves[move_count].promo   = NO_PIECE;

        move_count++;
    }

    while (p_att_e != BB_ZERO)
    {
        // square of a move
        int sq = BB_LSB(p_att_e);
        BB_UNSET_LSB(p_att_e);

        pl_moves[move_count].movedp = PAWN;
        pl_moves[move_count].captp  = piece_at_sq(sq);

        // we need this if for pawns.. unfortunately
        if (pl_moves[move_count].captp == NO_PIECE)
            pl_moves[move_count].captp = EN_PASSANTE;

        pl_moves[move_count].to_sq   = sq;
        pl_moves[move_count].from_sq = sq - (pawn_push_dir + EAST);
        pl_moves[move_count].promo   = NO_PIECE;

        move_count++;
    }

    while (p_att_w != BB_ZERO)
    {
        // square of a move
        int sq = BB_LSB(p_att_w);
        BB_UNSET_LSB(p_att_w);

        pl_moves[move_count].movedp = PAWN;
        pl_moves[move_count].captp  = piece_at_sq(sq);

        // we need this if just for pawns.. unfortunately
        if (pl_moves[move_count].captp == NO_PIECE)
            pl_moves[move_count].captp = EN_PASSANTE;

        pl_moves[move_count].to_sq   = sq;
        pl_moves[move_count].from_sq = sq - (pawn_push_dir + WEST);
        pl_moves[move_count].promo   = NO_PIECE;

        move_count++;
    }

    // --- KNIGHTS ---

    bitboard kn       = pieces(stm, KNIGHT);
    bitboard kn_moves = BB_ZERO;

    while (kn != BB_ZERO)
    {
        int kn_sq = BB_LSB(kn);
        BB_UNSET_LSB(kn);

        kn_moves = bb_knight_moves(kn_sq) & moveable_squares;

        while (kn_moves != BB_ZERO)
        {
            int sq = BB_LSB(kn_moves);
            BB_UNSET_LSB(kn_moves);

            pl_moves[move_count].movedp  = KNIGHT;
            pl_moves[move_count].captp   = piece_at_sq(sq);
            pl_moves[move_count].to_sq   = sq;
            pl_moves[move_count].from_sq = kn_sq;
            pl_moves[move_count].promo   = NO_PIECE;

            move_count++;
        }
    }

    // --- BISHOPS ---

    bitboard bsh       = pieces(stm, BISHOP);
    bitboard bsh_moves = BB_ZERO;

    while (bsh != BB_ZERO)
    {
        int bsh_sq = BB_LSB(bsh);
        BB_UNSET_LSB(bsh);

        bsh_moves = bb_bishop_moves(bsh_sq, occ) & moveable_squares;

        while (bsh_moves != BB_ZERO)
        {
            int sq = BB_LSB(bsh_moves);
            BB_UNSET_LSB(bsh_moves);

            pl_moves[move_count].movedp  = BISHOP;
            pl_moves[move_count].captp   = piece_at_sq(sq);
            pl_moves[move_count].to_sq   = sq;
            pl_moves[move_count].from_sq = bsh_sq;
            pl_moves[move_count].promo   = NO_PIECE;

            move_count++;
        }
    }

    // --- ROOKS ---

    bitboard rk       = pieces(stm, ROOK);
    bitboard rk_moves = BB_ZERO;

    while (rk != BB_ZERO)
    {
        int rk_sq = BB_LSB(rk);
        BB_UNSET_LSB(rk);

        rk_moves = bb_rook_moves(rk_sq, occ) & moveable_squares;

        while (rk_moves != BB_ZERO)
        {
            int sq = BB_LSB(rk_moves);
            BB_UNSET_LSB(rk_moves);

            pl_moves[move_count].movedp  = ROOK;
            pl_moves[move_count].captp   = piece_at_sq(sq);
            pl_moves[move_count].to_sq   = sq;
            pl_moves[move_count].from_sq = rk_sq;
            pl_moves[move_count].promo   = NO_PIECE;

            move_count++;
        }
    }

    // --- QUEENS ---

    bitboard qn = pieces(stm, QUEEN);

    bitboard qn_moves = BB_ZERO;

    while (qn != BB_ZERO)
    {
        int qn_sq = BB_LSB(qn);
        BB_UNSET_LSB(qn);

        qn_moves = bb_queen_moves(qn_sq, occ) & moveable_squares;

        while (qn_moves != BB_ZERO)
        {
            int sq = BB_LSB(qn_moves);
            BB_UNSET_LSB(qn_moves);

            pl_moves[move_count].movedp  = QUEEN;
            pl_moves[move_count].captp   = piece_at_sq(sq);
            pl_moves[move_count].to_sq   = sq;
            pl_moves[move_count].from_sq = qn_sq;
            pl_moves[move_count].promo   = NO_PIECE;

            move_count++;
        }
    }

    // --- KING ---

    bitboard kng = pieces(stm, KING);

    int kng_sq = BB_LSB(kng);

    bitboard kng_moves = bb_king_moves(kng_sq) & moveable_squares;

    while (kng_moves != BB_ZERO)
    {
        int sq = BB_LSB(kng_moves);
        BB_UNSET_LSB(kng_moves);

        pl_moves[move_count].movedp  = KING;
        pl_moves[move_count].captp   = piece_at_sq(sq);
        pl_moves[move_count].to_sq   = sq;
        pl_moves[move_count].from_sq = kng_sq;
        pl_moves[move_count].promo   = NO_PIECE;

        move_count++;
    }

    // --- CASTLING ---

    // this will give us the castle rights of the moving side only
    // so even if we are black, we still use "white" here in this section
    char moving_cr = ((castle_r) >> (stm * 2)) & WCR;

    // kingside
    if (moving_cr & WKS)
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
            pl_moves[move_count].movedp  = KING;
            pl_moves[move_count].captp   = NO_PIECE;
            pl_moves[move_count].to_sq   = kng_sq + (EAST * 2);
            pl_moves[move_count].from_sq = kng_sq;

            // how to indicate kingside castle
            pl_moves[move_count].promo = KING;

            move_count++;
        }
    }

    // Queenside, see comments above
    if (moving_cr & WQS)
    {
        int queenside_rooksq = stm == BLACK ? 56 : 0;

        bool spaces_free = (bb_rook_moves(queenside_rooksq, occ) & kng) > 0;

        bool attacked = sq_attacked(kng_sq + WEST, static_cast<COLOR>(!stm))
                     || sq_attacked(kng_sq + (WEST * 2), static_cast<COLOR>(!stm));

        if (spaces_free & !attacked)
        {
            pl_moves[move_count].movedp  = KING;
            pl_moves[move_count].captp   = NO_PIECE;
            pl_moves[move_count].to_sq   = kng_sq + (WEST * 2);
            pl_moves[move_count].from_sq = kng_sq;

            pl_moves[move_count].promo = QUEEN;

            move_count++;
        }
    }

    return pl_moves;
}

void Position::make_move(chess_move c)
{
    // TODO: if we double push a pawn- update enpassante
    //  I think the best way to do this is with a flag

    // move the moved piece
    BB_UNSET(pos_bbs[c.movedp], c.from_sq);
    BB_UNSET(pos_bbs[stm], c.from_sq);

    BB_SET(pos_bbs[c.movedp], c.to_sq);
    BB_SET(pos_bbs[stm], c.to_sq);

    // increment rev move counter (it will get reset if it needs to)
    rev_moves += 1;

    // moving pawns is not reversible
    if (c.movedp == PAWN)
        rev_moves = 0;

    // en passante is always cleared after a move
    pos_bbs[EN_PASSANTE] &= BB_ZERO;

    // unset the captured piece if there is one & reset rev move counter
    switch (c.captp)
    {
    case NO_PIECE:
        break;
    case EN_PASSANTE:
        rev_moves = 0;
        BB_UNSET(pos_bbs[PAWN], c.to_sq + PAWN_PUSH_DIR(static_cast<COLOR>(!stm)));
        BB_UNSET(pos_bbs[static_cast<COLOR>(!stm)], c.to_sq + PAWN_PUSH_DIR(static_cast<COLOR>(!stm)));
        break;
    default:
        rev_moves = 0;
        BB_UNSET(pos_bbs[c.captp], c.to_sq);
        BB_UNSET(pos_bbs[static_cast<COLOR>(!stm)], c.to_sq);
        break;
    }

    // increase full move counter if black
    full_moves += stm;

    // now it's the other side's turn
    stm = static_cast<COLOR>(!stm);
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
// size: ~41kb (8 bytes (bitboard) * 5184 entries)
static std::array<bitboard, 5184> BISHOP_MOVE_LOOKUP = {0};

// squares set might contain pieces that block bishop moves
static constexpr std::array<bitboard, 64> BISHOP_BLOCKER_MASK = {
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

// Where you can begin to lookup the move bb for a certain square
// for example, in a1 we have 63 (2^6 - 1) possible blocker combos,
// because there are 6 squares where blockers can be
// so for a2 we start at index 63 (64th element)
// max = 5184
static constexpr std::array<uint16_t, 64> BISHOP_MOVE_START_IDX = {
    0,    63,   94,   125,  156,  187,  218,  249,  312,  343,  374,  405,  436,  467,  498,  529,
    560,  591,  622,  749,  876,  1003, 1130, 1161, 1192, 1223, 1254, 1381, 1892, 2403, 2530, 2561,
    2592, 2623, 2654, 2781, 3292, 3803, 3930, 3961, 3992, 4023, 4054, 4181, 4308, 4435, 4562, 4593,
    4624, 4655, 4686, 4717, 4748, 4779, 4810, 4841, 4872, 4935, 4966, 4997, 5028, 5059, 5090, 5121};

bitboard bb_bishop_moves(square sq, const bitboard& blockers)
{
    bitboard blocker_key = BB_PEXT(blockers, BISHOP_BLOCKER_MASK[sq]);
    return BISHOP_MOVE_LOOKUP[BISHOP_MOVE_START_IDX[sq] + blocker_key];
}

void init_bishop_tables(void)
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

            int table_idx = BISHOP_MOVE_START_IDX[curr_sq] + blockers;

            BISHOP_MOVE_LOOKUP[table_idx] = moves;
        }
    }
}

// ------------------ ROOKS -----------------------

// lookup moves [idxlookup[sq] + PEXT(blockers)]
// we can make this more memory eff later (many elements are copies of eachother)
// size: ~800kb (8 bytes (bitboard) * 102337 entries)
static std::array<bitboard, 102337> ROOK_MOVE_LOOKUP = {0};

// squares set might contain pieces that block rook moves
static constexpr std::array<bitboard, 64> ROOK_BLOCKER_MASK = {
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
static constexpr std::array<uint32_t, 64> ROOK_MOVE_START_IDX = {
    0,     4095,  6142,  8189,  10236, 12283, 14330, 16377, 20472, 22519, 23542, 24565, 25588, 26611, 27634, 28657,
    30704, 32751, 33774, 34797, 35820, 36843, 37866, 38889, 40936, 42983, 44006, 45029, 46052, 47075, 48098, 49121,
    51168, 53215, 54238, 55261, 56284, 57307, 58330, 59353, 61400, 63447, 64470, 65493, 66516, 67539, 68562, 69585,
    71632, 73679, 74702, 75725, 76748, 77771, 78794, 79817, 81864, 85959, 88006, 90053, 92100, 94147, 96194, 98241};

bitboard bb_rook_moves(square sq, const bitboard& blockers)
{
    bitboard blocker_key = BB_PEXT(blockers, ROOK_BLOCKER_MASK[sq]);

    return ROOK_MOVE_LOOKUP[ROOK_MOVE_START_IDX[sq] + blocker_key];
}

void init_rook_tables(void)
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

            int table_idx = ROOK_MOVE_START_IDX[curr_sq] + blockers;

            ROOK_MOVE_LOOKUP[table_idx] = moves;
        }
    }
}

// ------------------QUEENS----------------------

bitboard bb_queen_moves(int sq, const bitboard& blockers)
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
bitboard bb_king_moves(int sq) { return king_lookup[sq]; }

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

bitboard bb_knight_moves(int sq) { return knight_lookup[sq]; }

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
