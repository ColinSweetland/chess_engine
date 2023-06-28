#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

#include "bitboard.hpp"
#include "engine.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "types.hpp"

void init(void)
{
    init_rook_table();
    init_bishop_table();
}

int main(void)
{
    srand(time(NULL));
    init();
    Engine::uci_loop();

    return 0;
}
