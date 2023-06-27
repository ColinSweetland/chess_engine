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
    init();
    Engine::uci_loop();

    // crash depth 5+
    // Position pos{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};

    return 0;
}
