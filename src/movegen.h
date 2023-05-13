#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "bitboard.h"
#include "game_state.h"

// parse move in long algebraic notation
chess_move parse_move(char *movestring);

// ----- KING MOVES -----
bitboard bb_king_moves(int sq, bitboard blockers);

// ----- PAWN MOVES -----
// 1. Attacks
bitboard bb_pawn_attacks_e(game_state *gs, COLOR side_to_move);
bitboard bb_pawn_attacks_w(game_state *gs, COLOR side_to_move);
// 2. Moves
bitboard bb_pawn_single_moves(game_state *gs, COLOR side_to_move);
bitboard bb_pawn_double_moves(game_state *gs, bitboard single_moves, COLOR side_to_move);

#endif