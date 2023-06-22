#include <cassert>
#include <cctype>

#include <sstream>

#include "bitboard.h"
// #include "movegen.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

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
            if (BB_IS_SET_AT(p.pos_bbs[WHITE], sq))
                piece_char = std::toupper(piece_char);

            out << ' ' << piece_char << ' ';
        }

        // print other game info
        switch (rank)
        {
        case (7):
            out << "\tFull Moves: " << p.full_moves << " Rev Moves: " << p.rev_moves;
            break;

        case (5):
            out << "\tCastling: ";
            if (p.castle_rights() & WKS)
                out << 'K';
            if (p.castle_rights() & WQS)
                out << 'Q';
            if (p.castle_rights() & BKS)
                out << 'k';
            if (p.castle_rights() & BQS)
                out << 'q';
            if (p.castle_rights() == 0)
                out << '-';
            break;

        case (3):
            out << "\tEn Passante Sq: ";

            if (enp_sq != -1)
                out << FILE_CHAR_FROM_SQ(enp_sq) << RANK_CHAR_FROM_SQ(enp_sq);
            else
                out << '-';

            break;

        case (1):
            out << "\t" << (p.side_to_move() ? "Black" : "White") << " to move";
            break;
        }
    }

    // print file label
    out << "\n\n\t a  b  c  d  e  f  g  h\n\n";

    return out;
}

// prints the gamestate in an extremely verbose way, for debugging only
void Position::dbg_print() const
{
    std::cout << "\n*** PAWNS ***\n";
    print_bitboard(PAWN);

    std::cout << "\n*** KNIGHT ***\n";
    print_bitboard(KNIGHT);

    std::cout << "\n*** BISHOP ***\n";
    print_bitboard(BISHOP);

    std::cout << "\n*** ROOK ***\n";
    print_bitboard(ROOK);

    std::cout << "\n*** QUEEN ***\n";
    print_bitboard(QUEEN);

    std::cout << "\n*** KING ***\n";
    print_bitboard(KING);

    std::cout << "\n*** EN PASSANTE ***\n";
    print_bitboard(EN_PASSANTE);

    std::cout << "\n*** WHITE ***\n";
    print_bitboard(WHITE);

    std::cout << "\n*** BLACK ***\n";
    print_bitboard(BLACK);

    std::cout << "*CASTLE RIGHTS*\n";
    std::cout << "WHITE KINGSIDE: " << ((castle_r & WKS) > 0) << "\n";
    std::cout << "WHITE QUEENSIDE: " << ((castle_r & WQS) > 0) << "\n";
    std::cout << "BLACK KINGSIDE: " << ((castle_r & BKS) > 0) << "\n";
    std::cout << "BLACK QUEENSIDE: " << ((castle_r & BQS) > 0) << "\n";

    std::cout << "\nEN PASSANTE TARGET: " << BB_LSB(pos_bbs[EN_PASSANTE]) << "\n";
    std::cout << "TO MOVE: " << stm << "\n";
    std::cout << "FULL MOVE COUNTER: " << full_moves << "\n";
    std::cout << "REVERSIBLE MOVE COUNTER: " << rev_moves << "\n";
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

bool Position::sq_attacked(square sq, COLOR attacking_color) const
{
    assert(sq >= 0 && sq < 64);
    // opposite of how att pawns move
    int      pawn_att_dir = -PAWN_PUSH_DIR(attacking_color);
    bitboard enemy_pawns  = pieces(attacking_color, PAWN);

    // enemy pawn attack from east side
    bitboard potential_pawn_att_sqs = BB_SQ(sq + pawn_att_dir + EAST) & ~BB_FILE_A;

    // from west
    potential_pawn_att_sqs |= BB_SQ(sq + pawn_att_dir + WEST) & ~BB_FILE_H;

    // check for pawn attackers
    if (potential_pawn_att_sqs & enemy_pawns)
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

bool Position::is_check(COLOR c) const
{
    square kng_square = BB_LSB(pieces(c, KING));

    assert(kng_square >= 0 && kng_square < 64);

    return sq_attacked(kng_square, static_cast<COLOR>(!c));
}

void Position::make_move(const ChessMove c)
{
    square orig_sq = c.get_origin_sq();
    square dest_sq = c.get_dest_sq();

    assert(orig_sq != dest_sq);

    PIECE moved_piece      = piece_at_sq(orig_sq);
    PIECE captured_piece   = NO_PIECE;
    PIECE after_move_piece = c.is_promo() ? c.get_promo_piece() : moved_piece;

    rev_move_data rmd{c, rev_moves, castle_r, captured_piece, pos_bbs[EN_PASSANTE]};

    int pushdir = PAWN_PUSH_DIR(stm);

    // increment rev move counter (will be reset later if it needs to)
    rev_moves += 1;
    // increase full moves (only if black)
    full_moves += stm;

    // en passante is always cleared after a move, or set if it was double push
    if (c.is_double_push())
        pos_bbs[EN_PASSANTE] = BB_SQ(orig_sq + pushdir);

    else
        pos_bbs[EN_PASSANTE] = BB_ZERO;

    // moving pawns is not reversible
    if (moved_piece == PAWN)
        rev_moves = 0;

    // moving king always unsets castle rights for moving side
    else if (moved_piece == KING)
        castle_r &= (stm ? WCR : BCR);

    // if we move rook from starting square, we must unset castle rights
    else if (moved_piece == ROOK)
    {
        square ks_rooksq = stm ? 63 : 7;
        square qs_rooksq = stm ? 56 : 0;

        if (orig_sq == ks_rooksq)
            castle_r &= (stm ? ~BKS : ~WKS);

        else if (orig_sq == qs_rooksq)
            castle_r &= (stm ? ~BQS : ~WQS);
    }

    // special moves section
    if (c.is_capture())
    {
        square cap_square = dest_sq;

        // for en_passante, the capture square is
        // different than the square the pawn ends up
        if (c.is_en_passante())
            cap_square -= pushdir;

        captured_piece = piece_at_sq(cap_square);

        rmd.captured_piece = captured_piece;

        // if we capture enemy rook from starting square, we must unset castle rights
        if (captured_piece == ROOK)
        {
            square enemy_ks_rooksq = stm ? 7 : 63;
            square enemy_qs_rooksq = stm ? 0 : 56;

            if (dest_sq == enemy_ks_rooksq)
                castle_r &= (stm ? ~WKS : ~BKS);

            else if (dest_sq == enemy_qs_rooksq)
                castle_r &= (stm ? ~WQS : ~BQS);
        }

        // remove the captured piece
        BB_UNSET(pos_bbs[captured_piece], cap_square);
        BB_UNSET(pos_bbs[!stm], cap_square);

        // captures are not reversible
        rev_moves = 0;
    }
    else if (c.get_flags() == ChessMove::KINGSIDE_CASTLE)
    {
        square rook_sq   = stm ? 63 : 7;
        square rook_dest = dest_sq + WEST;

        // we only have to move the rook, since we will move the king
        BB_UNSET(pos_bbs[ROOK], rook_sq);
        BB_UNSET(pos_bbs[stm], rook_sq);

        BB_SET(pos_bbs[ROOK], rook_dest);
        BB_SET(pos_bbs[stm], rook_dest);
    }
    else if (c.get_flags() == ChessMove::QUEENSIDE_CASTLE)
    {
        square rook_sq   = stm ? 56 : 0;
        square rook_dest = dest_sq + EAST;

        // we only have to move the rook, since we will move the king
        BB_UNSET(pos_bbs[ROOK], rook_sq);
        BB_UNSET(pos_bbs[stm], rook_sq);

        BB_SET(pos_bbs[ROOK], rook_dest);
        BB_SET(pos_bbs[stm], rook_dest);
    }

    // finally move the moved piece
    BB_UNSET(pos_bbs[moved_piece], orig_sq);
    BB_SET(pos_bbs[after_move_piece], dest_sq);

    BB_UNSET(pos_bbs[stm], orig_sq);
    BB_SET(pos_bbs[stm], dest_sq);

    // now it's the other side's turn
    stm = static_cast<COLOR>(!stm);

    // store reversible move data
    unmake_stack.push_back(rmd);
}

void Position::unmake_last(void)
{
    rev_move_data rmd = unmake_stack.back();
    unmake_stack.pop_back();

    ChessMove m = rmd.move;

    // restore unrestorable data
    pos_bbs[EN_PASSANTE] = rmd.enp_bb;
    rev_moves            = rmd.rev_move_clock;
    castle_r             = rmd.castle_r;

    PIECE after_move_piece = piece_at_sq(m.get_dest_sq());
    PIECE orig_piece       = m.is_promo() ? PAWN : after_move_piece;

    square dest_sq = m.get_dest_sq();
    square orig_sq = m.get_origin_sq();

    // swap stm back to who made the move
    stm = static_cast<COLOR>(!stm);

    // if the move was made by black, decrement
    full_moves -= stm;

    // remove piece from dest sq
    BB_UNSET(pos_bbs[stm], dest_sq);
    BB_UNSET(pos_bbs[after_move_piece], dest_sq);

    // restore piece to orig sq
    BB_SET(pos_bbs[stm], orig_sq);
    BB_SET(pos_bbs[orig_piece], orig_sq);

    if (m.is_capture())
    {
        square cap_sq = dest_sq;

        if (m.is_en_passante())
        {
            cap_sq -= PAWN_PUSH_DIR(stm);
        }

        // restore captured piece
        BB_SET(pos_bbs[!stm], cap_sq);
        BB_SET(pos_bbs[rmd.captured_piece], cap_sq);
    }
    else if (m.get_flags() == ChessMove::KINGSIDE_CASTLE)
    {
        square rook_orig = stm ? 63 : 7;
        square rook_dest = dest_sq + WEST;

        // restore rook to it's original square
        BB_UNSET(pos_bbs[ROOK], rook_dest);
        BB_UNSET(pos_bbs[stm], rook_dest);

        BB_SET(pos_bbs[ROOK], rook_orig);
        BB_SET(pos_bbs[stm], rook_orig);
    }
    else if (m.get_flags() == ChessMove::QUEENSIDE_CASTLE)
    {
        square rook_orig = stm ? 56 : 0;
        square rook_dest = dest_sq + EAST;

        // restore rook to it's original square
        BB_UNSET(pos_bbs[ROOK], rook_dest);
        BB_UNSET(pos_bbs[stm], rook_dest);

        BB_SET(pos_bbs[ROOK], rook_orig);
        BB_SET(pos_bbs[stm], rook_orig);
    }
}

bool Position::move_is_legal(ChessMove pseudo_legal)
{
    // square of side to move
    COLOR side_moving = side_to_move();

    make_move(pseudo_legal);

    square kng_sq = BB_LSB(pieces(side_moving, KING));

    // if the side that moved is in check, it's illegal
    if (sq_attacked(kng_sq, side_to_move()))
    {
        unmake_last();
        return false;
    }

    unmake_last();
    return true;
}

// Forsyth Edwards Notation is a common string based representation of a chess position
// https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

Position::Position(const char* fenstr) : pos_bbs{0}, castle_r{0}
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

        switch (fc)
        {
        // if we find a number: move num - 1 squares
        case '1' ... '8':
            bb_file += fc - '1';
            break;

        // end of row, move down rank and to first file
        case '/':
            bb_rank -= 1;
            bb_file = 0;
            break;

        // a letter indicates a piece
        case 'A' ... 'Z':
        case 'a' ... 'z':

            // set the piece color bitboard
            // black is lowercase - we use ! trick because isupper/lower can return any truthy int - we always want 1
            BB_SET(pos_bbs[!isupper(fc)], sq);

            // set the piece type bitboard
            // if fc is not a real piece char -> throw error
            assert(char_to_piece.count(tolower(fc)));

            BB_SET(pos_bbs[char_to_piece.at(tolower(fc))], sq);
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
                castle_r |= WKS;
                break;
            case ('Q'):
                castle_r |= WQS;
                break;
            case ('k'):
                castle_r |= BKS;
                break;
            case ('q'):
                castle_r |= BQS;
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
    fen >> rev_moves;

    // ---- FULL MOVE COUNTER ----
    fen >> full_moves;
}

str Position::FEN() const
{
    str fen_buf{15, ' '};

    std::ostringstream fen{fen_buf};

    int piece_color = 0;

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
            if (BB_IS_SET_AT(pieces(WHITE), sq))
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
    if (castle_r == NO_RIGHTS)
    {
        fen.put('-');
    }
    else
    {
        if (castle_r & WKS)
        {
            fen.put('K');
        }
        if (castle_r & WQS)
        {
            fen.put('Q');
        }
        if (castle_r & BKS)
        {
            fen.put('k');
        }
        if (castle_r & BQS)
        {
            fen.put('q');
        }
    }

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
        // FILE
        fen.put(FILE_CHAR_FROM_SQ(enp_sq));
        // RANK
        fen.put(RANK_CHAR_FROM_SQ(enp_sq));
    }

    // --- HALF/REVERSIBLE MOVES ---
    fen << ' ' << rev_moves << ' ';

    // --- FULL MOVE COUNTER ---
    fen << full_moves << "\n";

    return fen.str();
}
