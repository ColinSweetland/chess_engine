#include <iostream>

#include "bitboard.h"
#include "movegen.h"
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

void init(void)
{
    init_rook_tables();
    init_bishop_tables();
}

int main(void)
{
    init();

    // Position pos{"rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 1"};
    Position pos{"r3k2r/p6p/8/8/P7/bq6/P1P4P/B3K2R b KQkq - 0 1"};

    int mvcnt;

    auto ml = pos.pseudo_legal_moves(mvcnt);
    int  i  = 0;

    for (auto m : ml)
    {
        i++;
        std::cout << i << " " << m;

        if (i >= mvcnt)
            break;
    }

    std::cout << pos;
    // pos.dbg_print();
    std::cout << pos.FEN();

    print_bb(bb_queen_moves(17, pos.pieces()) & ~pos.pieces(pos.side_to_move()));

    return 0;
}
