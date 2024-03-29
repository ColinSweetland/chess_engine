#ifndef MOVEGEN_INCL
#define MOVEGEN_INCL

#include "./types/bitboard.hpp"

// forwards
class ChessMove;
class Position;
enum COLOR : int;

//--------------MOVES-------------------

std::ostream& operator<<(std::ostream& out, const ChessMove& move);

// ----- BISHOP MOVES-----
void init_bishop_table(void);

const bitboard& bb_bishop_moves(square sq, const bitboard& blockers);

// ----- ROOK MOVES-----
void init_rook_table(void);

const bitboard& bb_rook_moves(square sq, const bitboard& blockers);

// ----- QUEEN MOVES -----

bitboard bb_queen_moves(square sq, const bitboard& blockers);

// ----- KING MOVES -----
const bitboard& bb_king_moves(square sq);

//------ KNIGHT MOVES ---
const bitboard& bb_knight_moves(square sq);

// ----- PAWN MOVES -----
// 1. Attacks
bitboard bb_pawn_attacks_e(const bitboard& pawns, const bitboard& attackable, COLOR moving);
bitboard bb_pawn_attacks_w(const bitboard& pawns, const bitboard& attackable, COLOR moving);

// 2. Moves
bitboard bb_pawn_single_moves(const bitboard& pawns, const bitboard& blockers, COLOR side_to_move);

bitboard bb_pawn_double_moves(const bitboard& single_moves, const bitboard& blockers, COLOR side_to_move);

bitboard create_check_mask(const Position& pos);

#endif // MOVEGEN_INCL
