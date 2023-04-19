#include "bitboard.h"

typedef enum 
{
    WHITE,
    BLACK
} COLOR;

typedef enum 
{
    NONE_PIECE = -1,
    PAWN = 2, //start at two because index 0 & 1 represent color, the enum above
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
    bitboard bitboards[8];    
    char castle_rights;
    int en_passante_target;            // google it
    int reversible_move_counter;
    int full_move_counter;
    COLOR side_to_move;
} game_state;


typedef struct
{
    // these 3 are all that is needed to uniquely identify moves
    int from_square;
    int to_square;

    PIECE promotion; 

    /* We may want more info depending on our implementation of other stuff. 
    PIECE captured;
    int is_en_passante;
    int flags;
    */

} chess_move;

// parse move in long algebraic notation
chess_move parse_move(char *movestring);


// returns an initialized gamestate
// this is NOT the initial game position
// this is the default values set
game_state get_initialized_gamestate();

game_state gs_from_FEN(char *FEN);

char* FEN_from_gs(game_state *gs);

void dbg_print_gamestate(game_state *gs);

void print_gamestate(game_state *gs);
