#include "bitboard.h"

typedef enum 
{
    WHITE,
    BLACK
} COLOR;

typedef enum 
{
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING
} PIECE;

typedef enum
{
    WHITE_QUEENSIDE = 1,
    WHITE_KINGSIDE = 2,
    BLACK_QUEENSIDE = 4,
    BLACK_KINGSIDE = 8,
} CASTLE_RIGHTS;

typedef struct
{
    bitboard white_bitboards[6];    
    bitboard black_bitboards[6];
    int castle_rights;
    int EN_PASSANTE;            // google it
    int reversible_move_counter;
    int full_move_counter;
    COLOR side_to_move;
} game_state;

// returns an initialized gamestate
// this is NOT the initial game position
// this is the default values set
game_state get_initialized_gamestate();

game_state gs_from_FEN(char* FEN);

void dbg_print_gamestate(game_state gs);