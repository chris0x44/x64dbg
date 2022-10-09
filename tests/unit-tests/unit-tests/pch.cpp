#include "pch.h"

// Making use of pre-compiled header to drop compile time of empty file
// including catch_amalgamated.hpp from 1.1 sec to ~90ms

// Using the "force include" option in the project settings instead of
// explicitly including pch.h allows for checks for a clean include structure
// e.g. when using IWYU (include-what-you-use) checks
