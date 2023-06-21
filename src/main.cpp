#include <array>
#include <iostream>

#include "bitboard.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

#define MAX_UCI_INPUT_SIZE 1024

/*
void uci_engine_loop(void)
{
    char*  input      = (char*)malloc(MAX_UCI_INPUT_SIZE * sizeof(char));
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
*/

void init(void)
{
    init_rook_tables();
    init_bishop_tables();
}

int main(void)
{
    init();

    // Position pos{"rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1"};
    // Position pos{};
    Position pos{"r3k2r/p6p/8/8/2PbB3/8/P6P/R3K2R b KQkq c3 0 1"};

    move_list pl_moves = {};

    int mvc = pos.pseudo_legal_moves(pl_moves);

    std::cout << pos;

    for (int i = 0; i < mvc; i++)
        std::cout << i << ' ' << pl_moves[i];

    pos.make_move(pl_moves[4]);

    std::cout << pos;

    std::cout << pos.FEN();

    return 0;
}
