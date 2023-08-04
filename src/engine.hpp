#ifndef ENGINE_INCL
#define ENGINE_INCL

#include "chessmove.hpp"
#include "position.hpp"

namespace Engine
{

// interactive mode alters the engine's behavior in terminal
// to be more suited for humans rather than chess GUIS
void set_interactive();

void uci_loop();

} // namespace Engine

#endif // ENGINE_INCL