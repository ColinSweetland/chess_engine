#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "game_state.h"
#include "movegen.h"

#define MAX_UCI_INPUT_SIZE 1024

void uci_engine_loop(void)
{
    char *input = (char *)malloc(MAX_UCI_INPUT_SIZE * sizeof(char));
    size_t input_size = MAX_UCI_INPUT_SIZE;

    while (1)
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

void init(void)
{
    init_rook_tables();
    init_bishop_tables();
}

int main(void)
{
    init();

    // castling test
    game_state gs = gs_from_FEN("r3k2r/p6p/8/8/8/8/Q2n3P/R3K2R b KQkq - 0 1");

    // starter pos
    // game_state gs = gs_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // game_state gs = gs_from_FEN("rnbqkbnr/pp1ppppp/8/2p5/1P2P3/pppppppp/P1PP1PPP/RNBQKB1R w KQkq - 1 2");
    // uci_engine_loop();

    print_gamestate(&gs);

    chess_move *ml = (chess_move *)malloc(sizeof(chess_move) * 255);
    int movcnt = gen_all_moves(&gs, ml);

    for (int i = 0; i < movcnt; i++)
    {
        printf("%d : ", i);
        print_move(ml[i]);

        if (ml[i].promo != NONE_PIECE)
            printf("fromsq: %d, tosq, %d\n", ml[i].origin, ml[i].dest);
    }

    return 0;
}
