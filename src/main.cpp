#include <array>
#include <cassert>
#include <cstdint>
// #include <iostream>

// unix std header
#include <unistd.h>

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
    // set engine to interactive mode if stdout is a interactive terminal
    if (isatty(STDOUT_FILENO))
        Engine::set_interactive();

    srand(time(NULL));
    init();
    Engine::uci_loop();

    return 0;
}
