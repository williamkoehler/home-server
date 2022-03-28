#include "common.hpp"

extern "C" native::LibraryInformations GetLibraryInformations()
{
    return native::LibraryInformations{
        "debug-library",
        "Debug Scripts Library", // Name
        {1, 0, 0, 0},            // Version
        "MIT",                   // Lisence
        {"William KOEHLER"},     // Authors
        {},                      // Dependencies
    };
}