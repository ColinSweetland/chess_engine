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

PIECE Position::piece_at_sq(int sq) const
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

bool Position::sq_attacked(int sq, COLOR attacking_color) const
{
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

void Position::make_move(ChessMove c)
{
    // TODO: Handle castle moves
    // I think the best way to do this is with a flag
    square orig_sq = c.get_origin_sq();
    square dest_sq = c.get_dest_sq();

    PIECE moved_piece = piece_at_sq(orig_sq);
    int   flags       = c.get_flags();

    // increment rev move counter (it will get reset if it needs to)
    // also increase full moves (only if black)
    rev_moves += 1;
    full_moves += stm;

    // en passante is always cleared after a move, or set if it was double push
    if (c.is_double_push())
        pos_bbs[EN_PASSANTE] = BB_SQ(orig_sq + PAWN_PUSH_DIR(stm));
    else
        pos_bbs[EN_PASSANTE] = BB_ZERO;

    // moving pawns is not reversible
    if (moved_piece == PAWN)
        rev_moves = 0;

    // move the moved piece
    BB_UNSET(pos_bbs[moved_piece], orig_sq);
    BB_SET(pos_bbs[moved_piece], dest_sq);

    BB_UNSET(pos_bbs[stm], orig_sq);
    BB_SET(pos_bbs[stm], dest_sq);

    if (c.is_capture())
    {
        if (flags == ChessMove::flags::EP_CAP)
        {
            rev_moves = 0;
            BB_UNSET(pos_bbs[PAWN], dest_sq + PAWN_PUSH_DIR(static_cast<COLOR>(!stm)));
            BB_UNSET(pos_bbs[static_cast<COLOR>(!stm)], dest_sq + PAWN_PUSH_DIR(static_cast<COLOR>(!stm)));
        }
        else
        {
            rev_moves = 0;
            BB_UNSET(pos_bbs[piece_at_sq(dest_sq)], dest_sq);
            BB_UNSET(pos_bbs[static_cast<COLOR>(!stm)], dest_sq);
        }
    }
    // now it's the other side's turn
    stm = static_cast<COLOR>(!stm);
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
