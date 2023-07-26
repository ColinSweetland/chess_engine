#include <cassert>
#include <cctype>

#include <sstream>

#include "bitboard.hpp"
// #include "movegen.h"
#include "chessmove.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"

// formats castlerights to string like 'KQkq' or 'Kkq' or '-'
str Position::castle_right_str() const
{
    if (!has_cr(CR_ANY))
        return "-";

    str cr_str;

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
    square sq;
    char   piece_char;

    square enp_sq = BB_LSB(p.pos_bbs[EN_PASSANTE]);

    // print out 8 rows of pieces
    for (int rank = 8; rank >= 1; rank--)
    {

        out << "\n" << rank << "\t"; // print rank # label

        for (int file = 1; file <= 8; file++)
        {
            sq         = rankfile_to_sq(rank, file);
            piece_char = piece_to_char.at(p.piece_at_sq(sq));

            // if piece is white -> capital letter
            if (p.color_at_sq(sq) == WHITE)
                piece_char = std::toupper(piece_char);

            out << ' ' << piece_char << ' ';
        }

        // print other game info
        switch (rank)
        {
        case (7):
            out << "\tFull Moves: " << p.full_moves << " Rev Moves: " << p.rev_move_count;
            break;

        case (5):
            out << "\tCastling: " << p.castle_right_str();
            break;

        case (3):
            out << "\tEn Passante Sq: ";

            if (enp_sq != -1)
                out << SQ_TO_STR(enp_sq);
            else
                out << '-';

            break;

        case (1):
            out << "\t" << (piece_to_str.at(p.side_to_move())) << " to move";
            break;
        }
    }

    // print file label
    out << "\n\n\t a  b  c  d  e  f  g  h\n\n";

    out << "FEN: " << p.FEN() << "\n\n";

    return out;
}

void Position::dump_move_history() const
{
    for (auto st_info : state_info_stack)
        std::cout << st_info.prev_move << '\n';
}

void Position::remove_piece(COLOR c, PIECE p, square sq)
{
    assert(BB_IS_SET_AT(pos_bbs[c], sq));
    assert(BB_IS_SET_AT(pos_bbs[p], sq));
    assert(p != NO_PIECE);
    BB_UNSET(pos_bbs[c], sq);
    BB_UNSET(pos_bbs[p], sq);
}

void Position::place_piece(COLOR c, PIECE p, square sq)
{
    assert(!BB_IS_SET_AT(pos_bbs[c], sq));
    assert(!BB_IS_SET_AT(pos_bbs[p], sq));
    assert(p != NO_PIECE);
    BB_SET(pos_bbs[c], sq);
    BB_SET(pos_bbs[p], sq);
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
        if (BB_IS_SET_AT(pieces(p), sq))
        {
            return static_cast<PIECE>(p);
        }
    }

    return NO_PIECE;
}

COLOR Position::color_at_sq(square sq) const
{
    if (BB_IS_SET_AT(pieces(WHITE), sq))
        return WHITE;
    else if (BB_IS_SET_AT(pieces(BLACK), sq))
        return BLACK;
    else
        return NO_COLOR;
}

bool Position::sq_attacked(square sq, COLOR attacking_color) const
{
    assert(VALID_SQ(sq));

    // opposite of how att pawns move
    DIR pawn_att_dir = -PAWN_PUSH_DIR(attacking_color);

    bitboard potential_pawn_atts = GEN_SHIFT(BB_SQ(sq), pawn_att_dir + EAST) & ~BB_FILE_A;
    potential_pawn_atts |= GEN_SHIFT(BB_SQ(sq), pawn_att_dir + WEST) & ~BB_FILE_H;

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

    // rook moves from kings square
    bitboard rook_attkrs = bb_rook_moves(sq, pieces());

    // opposite pieces
    rook_attkrs &= pieces(attacking_color);

    // Rook or Queen
    rook_attkrs &= pieces(ROOK) | pieces(QUEEN);

    if (rook_attkrs)
    {
        return true;
    }

    // Bishop or Queen
    bitboard bsh_attkrs = bb_bishop_moves(sq, pieces());

    bsh_attkrs &= pieces(attacking_color);
    bsh_attkrs &= pieces(BISHOP) | pieces(QUEEN);

    return bsh_attkrs;
}

void Position::make_move(ChessMove move)
{
    assert(move.get_orig() != move.get_dest());

    // store reversible move data
    state_info_stack.emplace_back(move, rev_move_count, castle_r, pos_bbs[EN_PASSANTE]);

    // increment rev move counter (will be reset later if it needs to)
    rev_move_count += 1;

    // increase full moves (only if black)
    full_moves += stm;

    // en passante is set if it's a double push, else cleared
    if (move.is_double_push())
        pos_bbs[EN_PASSANTE] = BB_SQ(move.get_orig() + PAWN_PUSH_DIR(stm));
    else
        pos_bbs[EN_PASSANTE] = BB_ZERO;

    switch (move.get_moved_piece())
    {
    // moving pawns is not reversible
    case PAWN:
        rev_move_count = 0;
        break;

    // moving king always unsets castle rights for moving side
    case KING:
        remove_cr(stm ? CR_BKQS : CR_WKQS);
        break;

    // if we move rook from starting square, we must unset castle rights
    case ROOK:
    {
        square ks_rooksq = stm ? 63 : 7;
        square qs_rooksq = stm ? 56 : 0;

        if (move.get_orig() == ks_rooksq)
            remove_cr(stm ? CR_BKS : CR_WKS);

        else if (move.get_orig() == qs_rooksq)
            remove_cr(stm ? CR_BQS : CR_WQS);
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
            cap_square -= PAWN_PUSH_DIR(stm);
        }
        // if we capture enemy rook from starting square, we must unset castle rights
        if (move.get_captured_piece() == ROOK)
        {
            square enemy_ks_rooksq = stm ? 7 : 63;
            square enemy_qs_rooksq = stm ? 0 : 56;

            if (move.get_dest() == enemy_ks_rooksq)
                remove_cr(stm ? CR_WKS : CR_BKS);

            else if (move.get_dest() == enemy_qs_rooksq)
                remove_cr(stm ? CR_WQS : CR_BQS);
        }

        // remove the captured piece
        remove_piece(!stm, cap_piece, cap_square);

        // captures are not reversible
        rev_move_count = 0;
    }
    else if (move.is_castle())
    {
        square rook_sq;
        square rook_dest;
        // kingside castle
        if (FILE_FROM_SQ(move.get_dest()) == 7)
        {
            rook_sq   = stm ? 63 : 7;
            rook_dest = move.get_dest() + WEST;
        }
        else // queenside castle
        {
            rook_sq   = stm ? 56 : 0;
            rook_dest = move.get_dest() + EAST;
        }

        // we only have to move the rook, since we will move the king
        move_piece(stm, ROOK, rook_sq, rook_dest);
    }

    // finally move the moved piece
    move_and_change_piece(stm, move.get_moved_piece(), move.get_after_move_piece(), move.get_orig(), move.get_dest());

    // now it's the other side's turn
    stm = !stm;

    // *** update state_info ***

    state_info_stack.back().is_check = sq_attacked(BB_LSB(pieces(stm, KING)), !stm);
}

void Position::unmake_last()
{
    const state_info st_info = state_info_stack.back();
    state_info_stack.pop_back();

    const ChessMove move = st_info.prev_move;

    // restore unrestorable data
    pos_bbs[EN_PASSANTE] = st_info.prev_enp_bb;
    rev_move_count       = st_info.prev_rev_move_count;
    castle_r             = st_info.prev_castle_r;

    // swap stm back to who made the move
    stm = !stm;

    // if the move was made by black, decrement
    full_moves -= stm;

    // move piece back and maybe unpromote
    move_and_change_piece(stm, move.get_after_move_piece(), move.get_moved_piece(), move.get_dest(), move.get_orig());

    if (move.is_capture())
    {
        square cap_sq    = move.get_dest();
        PIECE  cap_piece = move.get_captured_piece();

        if (move.is_en_passante())
        {
            cap_sq -= PAWN_PUSH_DIR(stm);
            cap_piece = PAWN;
        }

        // restore captured piece
        place_piece(!stm, cap_piece, cap_sq);
    }
    else if (move.is_castle())
    {
        square rook_orig;
        square rook_dest;

        // kingside castle
        if (FILE_FROM_SQ(move.get_dest()) == 7)
        {
            rook_orig = stm ? 63 : 7;
            rook_dest = move.get_dest() + WEST;
        }
        // queenside castle
        else
        {
            rook_orig = stm ? 56 : 0;
            rook_dest = move.get_dest() + EAST;
        }

        // restore rook to it's original square
        move_piece(stm, ROOK, rook_dest, rook_orig);
    }
}

// try to make pseudo legal move.
// If move is legal, make the move and return true.
// If move is not legal, return false.
bool Position::try_make_move(const ChessMove pseudo_legal)
{
    make_move(pseudo_legal);
    // if the side that moved is in check, it's illegal
    if (sq_attacked(BB_LSB(pieces(!side_to_move(), KING)), side_to_move()))
    {
        unmake_last();
        return false;
    }

    return true;
}

// Forsyth Edwards Notation is a common string based representation of a chess position
// https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

Position::Position(const str fenstr) : pos_bbs{0}, castle_r{0}
{
    std::istringstream fen{fenstr};

    // Fen strings start at top left position
    int bb_file = 1;
    int bb_rank = 8;

    char fc;

    square sq = rankfile_to_sq(bb_rank, bb_file);

    // board position section
    while (sq != 7)
    {

        sq = rankfile_to_sq(bb_rank, bb_file);
        fen >> fc;

        // printf("SQ: %d R: %d F: %d C: %c\n", sq, bb_rank, bb_file, fc);

        switch (fc)
        {
        // if we find a number: move num - 1 squares
        case '1' ... '8':
            bb_file += fc - '1';

            // ugly hack -> if the last entry in the section is
            // a number, then break loop. seems to work
            if (bb_rank == 1 && bb_file >= 8)
                sq = 7;
            break;

        // end of row, move down rank and to first file
        case '/':
            bb_rank -= 1;
            bb_file = 0;
            break;

        // a letter indicates a piece
        case 'A' ... 'Z':
        case 'a' ... 'z':
            // if fc is not a real piece char -> throw error
            assert(char_to_piece.count(tolower(fc)));

            {
                // black is lowercase - we use ! because isupper/lower
                // can return any truthy int - we always want 1
                COLOR piece_color = static_cast<COLOR>(!isupper(fc));
                PIECE piece_type  = char_to_piece.at(tolower(fc));

                place_piece(piece_color, piece_type, sq);
            }

            break;

        default:
            // we should never have any other characters in the board section of the FEN string
            assert(false);
        }

        bb_file++;
    }

    // ---- which side moves ----

    fen >> fc;

    stm = fc == 'b' ? BLACK : WHITE;

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
                assert(false);
                break;
            }

            fen >> fc;
        }

        // ok to skip whitespace again
        fen >> std::skipws;
    }

    // ---- EN PASSANTE TARGET SQUARE ----
    // '-' indicates no enpassante available

    fen >> fc;

    if (fc != '-')
    {
        // FILE LETTER
        int enp_sq = fc - 'a';

        // RANK NUM
        fen >> fc;

        enp_sq += (fc - '1') * 8;

        pos_bbs[EN_PASSANTE] = BB_SQ(enp_sq);
    }

    // ---- REVERSIBLE or HALF MOVE COUNTER ----
    fen >> rev_move_count;

    // ---- FULL MOVE COUNTER ----
    fen >> full_moves;

    // now we must add a state info for the startpos (are these values okay?)
    state_info_stack.emplace_back(ChessMove{}, 0, castle_r, BB_ZERO);
    state_info_stack.back().is_check = sq_attacked(BB_LSB(pieces(stm, KING)), !stm);
}

str Position::FEN() const
{
    str fen_buf{15, ' '};

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
            char piece_char = piece_to_char.at(sq_piece);

            // if piece is white, make uppercase
            if (color_at_sq(sq) == WHITE)
            {
                piece_char = toupper(piece_char);
            }

            fen.put(piece_char);
        }

        // last file
        if (FILE_FROM_SQ(sq) == 8)
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
    fen << ' ';
    fen << piece_to_char.at(stm);
    fen << ' ';

    // --- CASTLE RIGHTS ---
    fen << castle_right_str();

    // --- EN PASSANTE ---

    fen.put(' ');

    int enp_sq = BB_LSB(pos_bbs[EN_PASSANTE]);

    // no enpassante available
    if (enp_sq < 0)
    {
        fen.put('-');
    }
    else
    {
        fen << SQ_TO_STR(enp_sq);
    }

    // --- HALF/REVERSIBLE MOVES ---
    fen << ' ' << rev_move_count << ' ';

    // --- FULL MOVE COUNTER ---
    fen << full_moves;

    return fen.str();
}
