#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "bitboard.h"
#include "game_state.h"


//--------------MOVES-------------------

typedef struct chess_move
{
    int from_sq;
    int to_sq;

    PIECE movedp;
    PIECE captp;
    PIECE promo; 
} chess_move;

void print_move(chess_move move);

int gen_all_moves(game_state *gs, chess_move* ml);

// parse move in long algebraic notation
chess_move *parse_move(char *movestring);

// ----- BISHOP MOVES-----
void init_bishop_tables();

bitboard bb_bishop_moves(int sq, bitboard blockers);

// ----- ROOK MOVES-----
void init_rook_tables();

bitboard bb_rook_moves(int sq, bitboard blockers);

// ----- QUEEN MOVES -----

bitboard bb_queen_moves(int sq, bitboard blockers);

// ----- KING MOVES -----
bitboard bb_king_moves(int sq);

//------ KNIGHT MOVES ---
bitboard bb_knight_moves(int sq);

// ----- PAWN MOVES -----
// 1. Attacks
bitboard bb_pawn_attacks_e(game_state *gs, COLOR side_to_move);
bitboard bb_pawn_attacks_w(game_state *gs, COLOR side_to_move);
// 2. Moves
bitboard bb_pawn_single_moves(bitboard pawns, bitboard blockers, COLOR side_to_move);

bitboard bb_pawn_double_moves(bitboard single_moves, bitboard blockers, COLOR side_to_move);

#endif