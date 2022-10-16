#include "patternfind.h"
#include <vector>
#include <algorithm>

using namespace std;

static inline bool isHex(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
}

static inline bool isWildcard(char ch)
{
   return ch == '?';
}

static inline bool isPaternChar(char ch)
{
   return isHex(ch) || isWildcard(ch);
}

static inline unsigned char hexchtoint(char ch)
{
    if(ch >= '0' && ch <= '9')
        return ch - '0';
    else if(ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else if(ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

static std::string stripspaces(std::string text)
{
   text.erase(std::remove(text.begin(), text.end(), ' '),text.end());
   return text;
}

static inline bool isValidPattern(const string& patterntext)
{
   return !patterntext.empty()
      && std::all_of(patterntext.cbegin(), patterntext.cend(), isPaternChar)
      && !std::all_of(patterntext.cbegin(), patterntext.cend(), isWildcard);
}

static PatternByte::PatternNibble createPatternNibble(char nibbleChar)
{
   return {hexchtoint(nibbleChar), isWildcard(nibbleChar)};
}

static PatternByte createPatternByte(char uppperNibble, char lowerNibble)
{
   return {createPatternNibble(uppperNibble), createPatternNibble(lowerNibble)};
}

bool patterntransform(const string & patterntext, vector<PatternByte> & pattern)
{
    pattern.clear();
    string formattext = stripspaces(patterntext);
    
    // reject invalid patterns
    if (!isValidPattern(formattext))
            return false;

    int len = (int)formattext.length();

    if(len % 2) //not a multiple of 2
    {
        formattext += '?';
        len++;
    }

    // format text is guaranteed to be a multiple of 2 at this point, so it is safe to
    // process two bytes at a time with each iteration
    size_t i = 0;
    while(i < formattext.size())
    {
       pattern.push_back(createPatternByte(formattext[i], formattext[i + 1]));
       i += 2;
    }

    return true;
}

static inline bool patternmatchbyte(unsigned char byte, const PatternByte & pbyte)
{
    int matched = 0;

    unsigned char n1 = (byte >> 4) & 0xF;
    if(pbyte.nibble[0].wildcard)
        matched++;
    else if(pbyte.nibble[0].data == n1)
        matched++;

    unsigned char n2 = byte & 0xF;
    if(pbyte.nibble[1].wildcard)
        matched++;
    else if(pbyte.nibble[1].data == n2)
        matched++;

    return (matched == 2);
}

size_t patternfind(const unsigned char* data, size_t datasize, const char* pattern, int* patternsize)
{
    string patterntext(pattern);
    vector<PatternByte> searchpattern;
    if(!patterntransform(patterntext, searchpattern))
        return -1;
    return patternfind(data, datasize, searchpattern);
}

size_t patternfind(const unsigned char* data, size_t datasize, unsigned char* pattern, size_t patternsize)
{
    if(patternsize > datasize)
        patternsize = datasize;
    for(size_t i = 0, pos = 0; i < datasize; i++)
    {
        if(data[i] == pattern[pos])
        {
            pos++;
            if(pos == patternsize)
                return i - patternsize + 1;
        }
        else if(pos > 0)
        {
            i -= pos;
            pos = 0; //reset current pattern position
        }
    }
    return -1;
}

static inline void patternwritebyte(unsigned char* byte, const PatternByte & pbyte)
{
    unsigned char n1 = (*byte >> 4) & 0xF;
    unsigned char n2 = *byte & 0xF;
    if(!pbyte.nibble[0].wildcard)
        n1 = pbyte.nibble[0].data;
    if(!pbyte.nibble[1].wildcard)
        n2 = pbyte.nibble[1].data;
    *byte = ((n1 << 4) & 0xF0) | (n2 & 0xF);
}

void patternwrite(unsigned char* data, size_t datasize, const char* pattern)
{
    vector<PatternByte> writepattern;
    string patterntext(pattern);
    if(!patterntransform(patterntext, writepattern))
        return;
    size_t writepatternsize = writepattern.size();
    if(writepatternsize > datasize)
        writepatternsize = datasize;
    for(size_t i = 0; i < writepatternsize; i++)
        patternwritebyte(&data[i], writepattern.at(i));
}

bool patternsnr(unsigned char* data, size_t datasize, const char* searchpattern, const char* replacepattern)
{
    size_t found = patternfind(data, datasize, searchpattern);
    if(found == -1)
        return false;
    patternwrite(data + found, datasize - found, replacepattern);
    return true;
}

size_t patternfind(const unsigned char* data, size_t datasize, const std::vector<PatternByte> & pattern)
{
    size_t searchpatternsize = pattern.size();
    for(size_t i = 0, pos = 0; i < datasize; i++) //search for the pattern
    {
        if(patternmatchbyte(data[i], pattern.at(pos))) //check if our pattern matches the current byte
        {
            pos++;
            if(pos == searchpatternsize) //everything matched
                return i - searchpatternsize + 1;
        }
        else if(pos > 0) //fix by Computer_Angel
        {
            i -= pos;
            pos = 0; //reset current pattern position
        }
    }
    return -1;
}