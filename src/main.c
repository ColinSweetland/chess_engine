#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_state.h"
#include "movegen.h"
#include "bitboard.h"

#define MAX_UCI_INPUT_SIZE 1024

void uci_engine_loop()
{
    char *input = malloc(MAX_UCI_INPUT_SIZE * sizeof(char));
    size_t input_size = MAX_UCI_INPUT_SIZE;

    while(1)
    {

        fgets(input, input_size, stdin);
        fflush(stdout);

        if (strncmp(input, "uci", 3) == 0)
        {
            printf("id name test_engine\n");
            printf("id author Colin Sweetland\n");
            printf("uciok\n");
        }

        else if (strncmp(input, "quit", 4) == 0)
        {
            free(input);
            exit(0);
        }

        else if (strncmp(input, "isready", 7) == 0)
        {
            printf("readyok\n");
        }
    }
}   

void init() 
{
    init_rook_tables();
    init_bishop_tables();
}

int main()
{
    init();

    //game_state gs = gs_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/1Pp5/1P2P3/5N2/P1PP1PPP/RNBQKB1R b KQkq - 1 2");
    // game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/pppppppp/P1PP1PPP/RNBQKB1R w KQkq - 1 2");
    //uci_engine_loop();

    print_gamestate(&gs);

    bitboard pmoves = bb_pawn_attacks_e(&gs, WHITE);

    print_bb(pmoves);

    return 0;
}