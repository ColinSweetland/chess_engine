#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

#include "bitboard.hpp"
#include "engine.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"

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
    init_rook_table();
    init_bishop_table();
}

int main(void)
{
    init();

    // Position pos{"rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1"};
    Position pos{};

    // Position pos{"rnbqkbnr/pppp1ppp/4p3/8/8/BP6/P1PPPPPP/RN1QKBNR b KQkq - 1 2"};

    // crash depth 5+
    // Position pos{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "};

    // weird crash
    // Position pos{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"};

    std::cout << pos;

    int depth = 4;

    perft_report_divided(pos, depth);

    return 0;
}
