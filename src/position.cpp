#include <cassert>
#include <cctype>

#include <sstream>

#include "./types/bitboard.hpp"
#include "chessmove.hpp"
#include "gameinfo.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types/pieces.hpp"
#include "util.hpp"
#include "zobrist.hpp"

// formats castlerights to string like 'KQkq' or 'Kkq' or '-'
std::string Position::castle_right_str() const
{
    if (!has_cr(CR_ANY))
        return "-";

    std::string cr_str;

    if (has_cr(CR_WKS))
        cr_str.push_back('K');
    if (has_cr(CR_WQS))
        cr_str.push_back('Q');
    if (has_cr(CR_BKS))
        cr_str.push_back('k');
    if (has_cr(CR_BQS))
        cr_str.push_back('q');

    return cr_str;
};

// the << operator will print the position, nicely formatted
std::ostream& operator<<(std::ostream& out, const Position& p)
{
    // print out 8 rows of pieces
    for (int rank = 8; rank >= 1; rank--)
    {

        out << "\n" << rank << "\t"; // print rank # label

        for (int file = 1; file <= 8; file++)
        {
            square sq = rf_to_sq(rank, file);
            out << ' ' << colorpiece_to_char({p.color_at_sq(sq), p.piece_at_sq(sq)}) << ' ';
        }

        // print other game info
        switch (rank)
        {
        case (7):
            out << "\tFull Moves: " << p.full_move_count() << " Rev Moves: " << p.rev_move_count();
            break;

        case (5):
            out << "\tCastling: " << p.castle_right_str();
            break;

        case (3):
            out << "\tEn Passante Sq: ";

            if (p.m_enp_sq != -1)
                out << sq_str(p.m_enp_sq);
            else
                out << '-';

            break;

        case (1):
            out << "\t" << (p.side_to_move() ? "BLACK" : "WHITE") << " to move";
            break;
        }
    }

    // print file label
    out << "\n\n\t a  b  c  d  e  f  g  h\n\n";

    out << "FEN: " << p.FEN() << "\n\n";

    return out;
}

bool Position::is_rep_draw() const
{
    auto info_stack_sz = m_state_info_stack.size();

    // number of times current pos is repeated in history
    int number_reps = 0;

    for (size_t i = 2; i <= info_stack_sz; i++)
    {
        if (i != 1
            && m_state_info_stack[info_stack_sz - i].pos_zhash == m_state_info_stack[info_stack_sz - 1].pos_zhash)
            number_reps++;

        if (!m_state_info_stack[info_stack_sz - i].prev_move_repeatable || number_reps == 3)
            break;
    }

    return number_reps >= 3;
}

void Position::dump_move_history() const
{
    for (auto st_info : m_state_info_stack)
        std::cout << st_info.prev_move << '\n';
}

void Position::dump_zhash() const { std::cout << util::pretty_int(m_curr_zhash) << '\n'; }

void Position::dump_rep_info() const
{
    auto info_stack_sz = m_state_info_stack.size();

    // number of times current pos is repeated in history
    int number_reps = 0;

    for (size_t i = 1; i <= info_stack_sz; i++)
    {
        std::cout << static_cast<unsigned int>(m_state_info_stack[info_stack_sz - i].pos_zhash) << '\n';

        if (i != 1
            && m_state_info_stack[info_stack_sz - i].pos_zhash == m_state_info_stack[info_stack_sz - 1].pos_zhash)
            number_reps++;

        if (!m_state_info_stack[info_stack_sz - i].prev_move_repeatable)
            break;
    }

    std::cout << "The current position was repeated " << number_reps << " times\n";
};

void Position::remove_piece(COLOR c, PIECE p, square sq)
{
    assert(bb_is_set_at_sq(m_color_bbs[c], sq));
    assert(bb_is_set_at_sq(m_piece_bbs[c][p], sq));
    assert(p != NO_PIECE);
    bb_unset_sq(m_color_bbs[c], sq);
    bb_unset_sq(m_piece_bbs[c][p], sq);

    m_curr_zhash ^= Zobrist::color_piece_on_sq(c, p, sq);
}

void Position::place_piece(COLOR c, PIECE p, square sq)
{
    assert(!bb_is_set_at_sq(m_color_bbs[c], sq));
    assert(!bb_is_set_at_sq(m_color_bbs[!c], sq));
    assert(!bb_is_set_at_sq(m_piece_bbs[c][p], sq));
    assert(!bb_is_set_at_sq(m_piece_bbs[!c][p], sq));
    assert(p != NO_PIECE);
    assert(is_valid(sq));
    bb_set_sq(m_color_bbs[c], sq);
    bb_set_sq(m_piece_bbs[c][p], sq);

    m_curr_zhash ^= Zobrist::color_piece_on_sq(c, p, sq);
}

void Position::move_piece(COLOR c, PIECE p, square orig, square dest)
{
    remove_piece(c, p, orig);
    place_piece(c, p, dest);
}

void Position::move_and_change_piece(COLOR c, PIECE orig_p, PIECE new_p, square orig_sq, square dest_sq)
{
    remove_piece(c, orig_p, orig_sq);
    place_piece(c, new_p, dest_sq);
}

PIECE Position::piece_at_sq(square sq) const
{
    for (int p = PAWN; p <= KING; p++)
    {
        if (bb_is_set_at_sq(pieces(static_cast<PIECE>(p)), sq))
        {
            return static_cast<PIECE>(p);
        }
    }

    return NO_PIECE;
}

COLOR Position::color_at_sq(square sq) const
{
    if (bb_is_set_at_sq(pieces(WHITE), sq))
        return WHITE;
    else if (bb_is_set_at_sq(pieces(BLACK), sq))
        return BLACK;
    else
        return NO_COLOR;
}

bool Position::sq_attacked(square sq, COLOR attacking_color) const
{
    assert(is_valid(sq));

    // opposite of how att pawns move
    DIR pawn_att_dir = -push_dir(attacking_color);

    bitboard potential_pawn_atts = gen_shift(bb_from_sq(sq), pawn_att_dir + EAST) & ~BB_FILE_A;
    potential_pawn_atts |= gen_shift(bb_from_sq(sq), pawn_att_dir + WEST) & ~BB_FILE_H;

    // check for pawn attackers
    if (potential_pawn_atts & pieces(attacking_color, PAWN))
    {
        return true;
    }

    // check for knight attackers
    if (bb_knight_moves(sq) & pieces(attacking_color, KNIGHT))
    {
        return true;
    }

    // check for king attacker
    if (bb_king_moves(sq) & pieces(attacking_color, KING))
    {
        return true;
    }

    // check for rook / queen attackers

    // rookwise attackers
    if (bb_rook_moves(sq, pieces()) & (pieces(attacking_color, ROOK) | pieces(attacking_color, QUEEN)))
    {
        return true;
    }

    // bishop attackers
    return bb_bishop_moves(sq, pieces()) & (pieces(attacking_color, BISHOP) | pieces(attacking_color, QUEEN));
}

void Position::update_checkers_bb()
{
    const bitboard occ    = pieces();
    const square   kng_sq = lsb(pieces(m_stm, KING));
    const COLOR    enemy  = !side_to_move();

    bitboard& checkers_bb = m_state_info_stack.back().checkers_bb;

    // pawn attackers (treating our king as a friendly pawn, with only pawns capturable)
    checkers_bb = bb_pawn_attacks_e(pieces(m_stm, KING), pieces(enemy, PAWN), m_stm);
    checkers_bb |= bb_pawn_attacks_w(pieces(m_stm, KING), pieces(enemy, PAWN), m_stm);

    // knight attackers
    checkers_bb |= bb_knight_moves(kng_sq) & pieces(enemy, KNIGHT);

    // rookwise attackers
    checkers_bb |= bb_rook_moves(kng_sq, occ) & (pieces(enemy, ROOK) | pieces(enemy, QUEEN));

    // bishopwise attackers
    checkers_bb |= bb_bishop_moves(kng_sq, occ) & (pieces(enemy, BISHOP) | pieces(enemy, QUEEN));
}

void Position::make_move(ChessMove move)
{
    assert(move.get_orig() != move.get_dest());

    // store reversible move data
    m_state_info_stack.emplace_back(move, m_rev_move_count, m_castle_r, m_enp_sq);

    // increment rev move counter (will be reset later if it needs to)
    m_rev_move_count += 1;

    // increase full moves (only if black)
    m_full_moves += m_stm;

    m_curr_zhash ^= Zobrist::castle_right(m_castle_r);

    if (is_valid(m_enp_sq))
        m_curr_zhash ^= Zobrist::ep_square(m_enp_sq);

    // en passante is set if it's a double push, else cleared
    if (move.is_double_push())
    {
        m_enp_sq = move.get_orig() + push_dir(m_stm);
        m_curr_zhash ^= Zobrist::ep_square(m_enp_sq);
    }
    else
        m_enp_sq = -1;

    switch (move.get_moved_piece())
    {
    // moving pawns is not reversible
    case PAWN:
        m_rev_move_count                               = 0;
        m_state_info_stack.back().prev_move_repeatable = false;
        break;

    // moving king always unsets castle rights for moving side
    case KING:
        remove_cr(m_stm ? CR_BKQS : CR_WKQS);
        break;

    // if we move rook from starting square, we must unset castle rights
    case ROOK:
    {
        square ks_rooksq = m_stm ? 63 : 7;
        square qs_rooksq = m_stm ? 56 : 0;

        if (move.get_orig() == ks_rooksq)
            remove_cr(m_stm ? CR_BKS : CR_WKS);

        else if (move.get_orig() == qs_rooksq)
            remove_cr(m_stm ? CR_BQS : CR_WQS);
        break;
    }
    // no other piece needs special treatment
    default:
        break;
    }

    // special moves section
    if (move.is_capture())
    {
        square cap_square = move.get_dest();
        PIECE  cap_piece  = move.get_captured_piece();

        // for en_passante, the capture square is
        // different than the square the pawn ends up
        if (cap_piece == EN_PASSANTE)
        {
            cap_piece = PAWN;
            cap_square -= push_dir(m_stm);
        }
        // if we capture enemy rook from starting square, we must unset castle rights
        if (move.get_captured_piece() == ROOK)
        {
            square enemy_ks_rooksq = m_stm ? 7 : 63;
            square enemy_qs_rooksq = m_stm ? 0 : 56;

            if (move.get_dest() == enemy_ks_rooksq)
                remove_cr(m_stm ? CR_WKS : CR_BKS);

            else if (move.get_dest() == enemy_qs_rooksq)
                remove_cr(m_stm ? CR_WQS : CR_BQS);
        }

        // remove the captured piece
        remove_piece(!m_stm, cap_piece, cap_square);

        // captures are not reversible
        m_rev_move_count                               = 0;
        m_state_info_stack.back().prev_move_repeatable = false;
    }
    else if (move.is_castle())
    {
        square rook_sq;
        square rook_dest;
        // kingside castle
        if (file_num(move.get_dest()) == 7)
        {
            rook_sq   = m_stm ? 63 : 7;
            rook_dest = move.get_dest() + WEST;
        }
        else // queenside castle
        {
            rook_sq   = m_stm ? 56 : 0;
            rook_dest = move.get_dest() + EAST;
        }

        // we only have to move the rook, since we will move the king
        move_piece(m_stm, ROOK, rook_sq, rook_dest);

        // castling is not repeatable for the purpose of repitition draw, but NOT for the 50 move rule (why?)
        m_state_info_stack.back().prev_move_repeatable = false;
    }

    // finally move the moved piece
    move_and_change_piece(m_stm, move.get_moved_piece(), move.get_after_move_piece(), move.get_orig(), move.get_dest());

    // now it's the other side's turn
    m_stm = !m_stm;
    m_curr_zhash ^= Zobrist::black_to_move();

    m_curr_zhash ^= Zobrist::castle_right(m_castle_r);

    // *** update state_info ***
    m_state_info_stack.back().pos_zhash = m_curr_zhash;
    update_checkers_bb();

    if (m_state_info_stack.back().prev_castle_r != m_castle_r)
    {
        m_state_info_stack.back().prev_move_repeatable = false;
    }
}

void Position::unmake_last()
{
    const state_info st_info = m_state_info_stack.back();
    m_state_info_stack.pop_back();

    const ChessMove move = st_info.prev_move;

    m_curr_zhash ^= Zobrist::castle_right(m_castle_r);
    if (is_valid(m_enp_sq))
        m_curr_zhash ^= Zobrist::ep_square(m_enp_sq);

    // restore unrestorable data
    m_enp_sq         = st_info.prev_enp_sq;
    m_rev_move_count = st_info.prev_rev_move_count;
    m_castle_r       = st_info.prev_castle_r;

    m_curr_zhash ^= Zobrist::castle_right(m_castle_r);
    if (is_valid(m_enp_sq))
        m_curr_zhash ^= Zobrist::ep_square(m_enp_sq);

    // swap stm back to who made the move
    m_stm = !m_stm;
    m_curr_zhash ^= Zobrist::black_to_move();

    // if the move was made by black, decrement
    m_full_moves -= m_stm;

    // move piece back and maybe unpromote
    move_and_change_piece(m_stm, move.get_after_move_piece(), move.get_moved_piece(), move.get_dest(), move.get_orig());

    if (move.is_capture())
    {
        square cap_sq    = move.get_dest();
        PIECE  cap_piece = move.get_captured_piece();

        if (move.is_en_passante())
        {
            cap_sq -= push_dir(m_stm);
            cap_piece = PAWN;
        }

        // restore captured piece
        place_piece(!m_stm, cap_piece, cap_sq);
    }
    else if (move.is_castle())
    {
        square rook_orig;
        square rook_dest;

        // kingside castle
        if (file_num(move.get_dest()) == 7)
        {
            rook_orig = m_stm ? 63 : 7;
            rook_dest = move.get_dest() + WEST;
        }
        // queenside castle
        else
        {
            rook_orig = m_stm ? 56 : 0;
            rook_dest = move.get_dest() + EAST;
        }

        // restore rook to it's original square
        move_piece(m_stm, ROOK, rook_dest, rook_orig);
    }
}

// try to make pseudo legal move.
// If move is legal, make the move and return true.
// If move is not legal, return false.
bool Position::try_make_move(const ChessMove pseudo_legal)
{
    make_move(pseudo_legal);
    // if the side that moved is in check, it's illegal
    if (sq_attacked(lsb(pieces(!side_to_move(), KING)), side_to_move()))
    {
        unmake_last();
        return false;
    }

    return true;
}

// Forsyth Edwards Notation is a common string based representation of a chess position
// https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

Position::Position(const std::string fenstr) : m_piece_bbs{{0}}, m_color_bbs{0}, m_curr_zhash{0}, m_castle_r{0}
{
    std::istringstream fen{fenstr};

    // Fen strings start at top left position
    int file = 1;
    int rank = 8;

    char fc;

    square sq = rf_to_sq(rank, file);

    // board position section
    while (sq != 7)
    {
        sq = rf_to_sq(rank, file);
        fen >> fc;

        switch (fc)
        {
        // if we find a number: move num - 1 squares
        case '1' ... '8':
            file += fc - '1';

            // ugly hack -> if the last entry in the section is
            // a number, then break loop. seems to work
            if (rank == 1 && file >= 8)
                sq = 7;

            break;

        // end of row, move down rank and to first file
        case '/':
            rank -= 1;
            file = 0;
            break;

        // a letter indicates a piece
        case 'A' ... 'Z':
        case 'a' ... 'z':
        {
            ColorPiece char_color_piece = char_to_colorpiece(fc);
            assert(char_color_piece.piece != NO_PIECE && char_color_piece.color != NO_COLOR);
            place_piece(char_color_piece.color, char_color_piece.piece, sq);
        }
        break;

        default:
            // we should never have any other characters in the board section of the FEN string
            assert(false);
        }

        file++;
    }

    // ---- which side moves ----

    fen >> fc;

    if (fc == 'b')
    {
        m_stm = BLACK;
        m_curr_zhash ^= Zobrist::black_to_move();
    }
    else if (fc == 'w')
    {
        m_stm = WHITE;
    }
    else
    {
        std::cerr << "FEN error: found '" << fc << "' when side to move was expected ('w' or 'b')\n";
        exit(1);
    }

    // ---- castling rights ----
    // '-' indicates nobody can castle

    fen >> fc;

    if (fc != '-')
    {
        // space indicates end of section

        // don't skip whitespace for now (so we know when we are done castle rights)
        fen >> std::noskipws;

        while (fc != ' ')
        {
            switch (fc)
            {
            case ('K'):
                give_cr(CR_WKS);
                break;
            case ('Q'):
                give_cr(CR_WQS);
                break;
            case ('k'):
                give_cr(CR_BKS);
                break;
            case ('q'):
                give_cr(CR_BQS);
                break;
            default:
                std::cerr << "FEN error: Found unexpected char '" << fc << "'"
                          << " in castle rights\n";
                exit(1);
                break;
            }

            fen >> fc;
        }

        // ok to skip whitespace again
        fen >> std::skipws;
    }

    m_curr_zhash ^= Zobrist::castle_right(m_castle_r);

    // ---- EN PASSANTE TARGET SQUARE ----
    // '-' indicates no enpassante available

    fen >> fc;

    if (fc != '-')
    {
        // FILE LETTER
        m_enp_sq = fc - 'a';

        // RANK NUM
        fen >> fc;
        m_enp_sq += (fc - '1') * 8;

        m_curr_zhash ^= Zobrist::ep_square(m_enp_sq);
    }
    else
    {
        m_enp_sq = -1;
    }

    // ---- REVERSIBLE or HALF MOVE COUNTER ----
    fen >> m_rev_move_count;

    // ---- FULL MOVE COUNTER ----
    fen >> m_full_moves;

    // now we must add a state info for the startpos (are these values okay?)
    m_state_info_stack.emplace_back(ChessMove{}, 0, m_castle_r, BB_ZERO);
    m_state_info_stack.back().pos_zhash = m_curr_zhash;
    update_checkers_bb();
}

std::string Position::FEN() const
{
    std::string fen_buf{15, ' '};

    std::ostringstream fen{fen_buf};

    PIECE sq_piece;
    int   empty_space_count = 0;

    // ---- BOARD SECTION ----

    // through each bit board index - upper left to lower right
    for (int sq = 56; sq >= 0; sq++)
    {
        sq_piece = piece_at_sq(sq);

        if (sq_piece == NO_PIECE)
        {
            empty_space_count++;
        }
        else
        {
            // if empty space came before this piece, write the number
            if (empty_space_count)
            {
                fen.put(empty_space_count + '0');
                empty_space_count = 0;
            }

            fen.put(colorpiece_to_char({color_at_sq(sq), sq_piece}));
        }

        // last file
        if (file_num(sq) == 8)
        {
            // if we have empty space at the end of the line..
            // we must write it, since it can't carry to the next line.
            if (empty_space_count)
            {
                fen.put(empty_space_count + '0');
                empty_space_count = 0;
            }

            // add / which means end of row, except on the last rank (rank 1)
            if (sq != 7)
            {
                fen << '/';
            }

            // shift bb index to next row, first entry
            sq -= 16;
        }
    }

    // --- SIDE TO MOVE ---
    fen.put(' ');
    fen.put(m_stm ? 'b' : 'w');
    fen.put(' ');

    // --- CASTLE RIGHTS ---
    fen << castle_right_str();

    // --- EN PASSANTE ---

    fen.put(' ');

    // no enpassante available
    if (m_enp_sq < 0)
    {
        fen.put('-');
    }
    else
    {
        fen << sq_str(m_enp_sq);
    }

    // --- HALF/REVERSIBLE MOVES ---
    fen << ' ' << m_rev_move_count << ' ';

    // --- FULL MOVE COUNTER ---
    fen << m_full_moves;

    return fen.str();
}
