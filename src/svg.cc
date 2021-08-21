#include "svg.hpp"
#include <stdio.h>
#include <assert.h>
#include <cz/string.hpp>
#include <vector>

using namespace cz;
using namespace std;

struct Tag {
    enum {selfclosing, open, close} closetype;
    String name;
    Vector<String> keys;
    Vector<String> values;
};

char buffer[0x200000] = {0};

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

    
bool ReadSVGFile(String path, Svg *Result)
// Read / Parse an SVG file and return the result in 
// *Result.
// returns whether or not the operation was successful.
{
    FILE *svgFile = fopen(path.buffer(), "r");
    if (svgFile == NULL)
        return false;

    int size = fread(&buffer, sizeof(char), sizeof(buffer), svgFile);
    if (size == sizeof(buffer))
        assert("File too big!");




    return true;
}