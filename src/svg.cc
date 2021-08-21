#include "svg.hpp"
#include <stdio.h>
#include <assert.h>
#include <cz/buffer_array.hpp>
#include <vector>

using namespace cz;
using namespace std;



char buffer[0x200000] = {0};

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

    
bool ReadSVGFile(Str path, Svg *Result)
// Read / Parse an SVG file and return the result in 
// *Result.
// returns whether or not the operation was successful.
{
    FILE *svgFile = fopen(path.buffer, "r");
    if (svgFile == NULL)
        return false;

    int size = fread(&buffer, sizeof(char), sizeof(buffer), svgFile);
    if (size == sizeof(buffer))
        assert("File too big!");

    Buffer_Array TheAllocator = {0};
    TheAllocator.init();
    Vector<Tag> tags = {0};
    for (size_t i = 0; i < size;) {
        Tag TheTag = {};
        printf("%c\n", buffer[i]);
        assert(buffer[i] == '<');
        i++;
        if (buffer[i] == '/') {
            TheTag.closetype = CloseType::selfclosing;
            i++;
        }
        
        size_t current_allocated_size = 0x1000;
        MemSlice name = {TheAllocator.allocator().alloc({current_allocated_size, 1}), current_allocated_size};
        int tag_name_size = 0;
        while (buffer[i] != ' ' && i < size) {
            tag_name_size++;
            if (tag_name_size > name.size) {
                TheAllocator.realloc(&TheAllocator, name, {name.size * 2, 1});
            }
            i++;
        }

        while (i < size && buffer[i] != '<') {
            i++;
        }
    }

    return true;
}