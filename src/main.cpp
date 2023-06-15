#include <iostream>

#include "bitboard.h"
#include "position.h"
#include "types.h"
// #include "movegen.h"

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

/*
void init(void)
{
    init_rook_tables();
    init_bishop_tables();
}
*/

int main(void)
{
    // init();

    Position pos{"rnbqkbnr/pp1ppppp/8/2p5/1P2P3/8/P1PP1PPP/RNBQKB1R b KQkq f6 1 2"};
    // Position pos{};

    // uci_engine_loop();

    std::cout << pos;
    std::cout << pos.FEN();

    pos.dbg_print();

    return 0;
}
