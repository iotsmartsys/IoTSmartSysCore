#include "Contracts/Display/Screen.h"

namespace iotsmartsys::core
{
    namespace
    {
        NoOpScreenConsole defaultConsole;
    }

    IScreenConsole *Screen::_console = nullptr;

    IScreenConsole &Screen::get()
    {
        return (_console != nullptr) ? *_console : static_cast<IScreenConsole &>(defaultConsole);
    }
} // namespace iotsmartsys::core
