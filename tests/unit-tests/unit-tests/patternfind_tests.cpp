#include "framework/catch_amalgamated.hpp"

// ATTENTION: Using include to "copy" sources of the system under test
// into this file, instead of adding it to the project.
// Advantage: static functions become accessible in case they need to
// be tested
#include "dbg/patternfind.cpp"



// comparison operators to make tests more readable

bool operator==(const PatternByte::PatternNibble & lhs, const PatternByte::PatternNibble & rhs)
{
    return lhs.wildcard == rhs.wildcard
        && (lhs.wildcard || lhs.data == rhs.data);
}


bool operator==(const PatternByte & lhs, const PatternByte & rhs)
{
    return lhs.nibble[0] == rhs.nibble[0] && lhs.nibble[1] == rhs.nibble[1];
}


// Test cases

TEST_CASE("patterntransform: pattern validation")
{
    std::vector<PatternByte> patternBytes;

    const auto hasPatternWildcards = [](const PatternByte & entry)
    {
        return entry.nibble[0].wildcard || entry.nibble[1].wildcard;
    };

    SECTION("Empty pattern, expects failure")
    {
        REQUIRE(false == patterntransform("", patternBytes));
    }

    SECTION("Failing input, existing elements in PatternBytes are removed")
    {
        patternBytes.push_back({});
        REQUIRE(false == patterntransform("", patternBytes));
        REQUIRE(patternBytes.empty());
    }

    SECTION("Invalid pattern character, expects failure")
    {
        REQUIRE(false == patterntransform("t", patternBytes));
    }

    SECTION("Only spaces as pattern, expects failure")
    {
        REQUIRE(false == patterntransform("  ", patternBytes));
    }

    SECTION("Valid one byte pattern with wildcard, single entry")
    {
        PatternByte expectedPattern{{{true, 0}, {false, 1}}};
        REQUIRE(true == patterntransform("1?", patternBytes));

        REQUIRE(1 == patternBytes.size());
        REQUIRE(expectedPattern == patternBytes.front());
    }

    SECTION("Odd number of characters in pattern, behaves as if missing character is wildcard")
    {
        std::vector<PatternByte> explicitWildcardPattern;
        std::vector<PatternByte> autoAppendedWildcardPattern;
        const bool expectedResult = patterntransform("1?", explicitWildcardPattern);

        REQUIRE(expectedResult == patterntransform("1", autoAppendedWildcardPattern));
        REQUIRE(explicitWildcardPattern == autoAppendedWildcardPattern);
    }

    SECTION("Valid pattern without wildcards, matching number of entries")
    {
        REQUIRE(true == patterntransform("12 ef", patternBytes));
        REQUIRE(2 == patternBytes.size());

        bool hasWildcards = std::any_of(patternBytes.begin(), patternBytes.end(), hasPatternWildcards);
        REQUIRE(false == hasWildcards);
    }

    SECTION("Only wildcard pattern is rejected")
    {
        REQUIRE(false == patterntransform("???", patternBytes));
    }
}
