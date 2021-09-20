#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "svg.hpp"
#include <cz/string.hpp>
#include <cz/directory.hpp>
#include <cz/heap.hpp>
#include <iostream>
using namespace cz;

#define MAX_USER_LEN 200
#define ZWJ          0x200D

// References
////////////////////////////////////////////////////////////////////////////////
//
// [REF:UTF-8 Encoding]
//    - https://en.wikipedia.org/wiki/UTF-8#Encoding
//
// [REF:SVG Paths]
//    - https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
//
////////////////////////////////////////////////////////////////////////////////

typedef struct _uclist {
    uint32_t        NumCodePoints;
    uint64_t       *CodePoints;
    Svg             TheSvg;
} UnicodeChars;


void fail(const char *);
Svg eat(Svg Eater, Svg Eaten);

int main(int argc, char **argv) {
    // Read user input
    char     UserString[MAX_USER_LEN] = {0};
    uint32_t StringLength             = 0;
    for (int Index = 0; Index < MAX_USER_LEN - 1; Index++) {
        int Result = fgetc(stdin);
        if (Result == EOF)
            break;
        UserString[StringLength++] = Result;
    }

    // Convert each utf-8 encoding to a unicode code point
    uint64_t CodePoints[MAX_USER_LEN] = {0};
    uint32_t CodePointNum             = 0;
    {
        uint32_t StringOffset  = 0;
        while (StringOffset < StringLength) {
            // Switch on the length encoded in the first byte
            char FirstByte = UserString[StringOffset++];
            if ((FirstByte & 0x80) == 0) {
                // Single byte encoding
                // 0b0xxxxxxx
                CodePoints[CodePointNum++] = FirstByte;
            } else if ((FirstByte & 0xE0) == 0xC0) {
                // Two byte encoding
                // 0b110xxxxx 0b10xxxxxx
                if (StringOffset == StringLength)
                    fail("Out of bounds (2 byte)");
                
                char SecondByte = UserString[StringOffset++];
                CodePoints[CodePointNum++] = (((uint64_t) FirstByte  & 0x1F) << 6) |
                                             (((uint64_t) SecondByte & 0x3F));
            } else if ((FirstByte & 0xF0) == 0xE0) {
                // Three byte encoding
                // 0b1110xxxx 0b10xxxxxx 0b10xxxxxx
                if (StringOffset == StringLength)
                    fail("Out of bounds (3 byte)");
                char SecondByte = UserString[StringOffset++];
                if (StringOffset == StringLength)
                    fail("Out of bounds (3 byte)");
                char ThirdByte = UserString[StringOffset++];

                CodePoints[CodePointNum++] = (((uint64_t) FirstByte  & 0x0F) << 12) |
                                             (((uint64_t) SecondByte & 0x3F) << 6 ) |
                                             (((uint64_t) ThirdByte  & 0x3F));
            } else if ((FirstByte & 0xF8) == 0xF0) {
                // Four byte encoding
                // 0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx
                if (StringOffset == StringLength)
                    fail("Out of bounds (4 byte)");
                char SecondByte = UserString[StringOffset++];
                if (StringOffset == StringLength)
                    fail("Out of bounds (4 byte)");
                char ThirdByte = UserString[StringOffset++];
                if (StringOffset == StringLength)
                    fail("Out of bounds (4 byte)");
                char FourthByte = UserString[StringOffset++];

                CodePoints[CodePointNum++] = (((uint64_t) FirstByte  & 0x07) << 18) |
                                             (((uint64_t) SecondByte & 0x3F) << 12) |
                                             (((uint64_t) ThirdByte  & 0x3F) << 6 ) |
                                             (((uint64_t) FourthByte & 0x3F));
            } else {
                fail("Invalid code point");
            }

        }
    } 

    uint64_t    *Start  = &CodePoints[0];
    uint32_t     EmojiLength = 0;
    cz::Vector<UnicodeChars> List   = {};
    List.reserve(cz::heap_allocator(), CodePointNum);
    for (uint32_t Index = 0; Index < CodePointNum; Index++) {
        if (CodePoints[Index] == ZWJ || CodePoints[Index] == '\n') {
            if (EmojiLength == 0) {
                Start = &CodePoints[Index + 1];
                std::cerr << "Empty segment at index " << Index << std::endl;
                exit(1);  
            }
            List.push({EmojiLength, Start, {}});
            Start = &CodePoints[Index + 1];
            EmojiLength = 0;
        } else {
            EmojiLength++;
        }
    }

    if (List.len() == 0 && EmojiLength == 0) {
        std::cerr << "No input" << std::endl;
    }

    // ignore trailing null byte
    if (EmojiLength != 0 && *Start != '\0') {
        List.push({EmojiLength, Start, {}});
    }


    cz::Allocator TheAlloc = cz::heap_allocator();
    cz::Directory_Iterator iterator = {};
    cz::String Result = {};
    iterator.init("../twemoji-svg", TheAlloc, &Result);
    
    int n = 0;
    while (iterator.advance(TheAlloc, &Result).is_ok() && Result.len() != 0) {
        Result.set_len(0);
        n++;
    }
    // Read in the base emoji images
    for (size_t i = 0; i < List.len(); i++) {
        char hex[0x28] = {0};
        snprintf(&hex[0], 27, "../twemoji-svg/%lx.svg", List[i].CodePoints[0]);
        std::cout << hex << std::endl;
        ReadSVGFile(hex, &List[i].TheSvg);
    }

    Point offset = {0.0,0.0};
    // Combine them based on whatever the user requested
    for (size_t i = List.len() - 1; i > 0; i--) {
        List[i - 1].TheSvg = eat(List[i - 1].TheSvg, List[i].TheSvg);
    }

    // Output the new images
    cz::String Output = {};
    List[0].TheSvg.output(&Output);
    Output.null_terminate();
    FILE *file = fopen("out.svg", "w");
    fprintf(file, "%s", Output.buffer());
}

Svg eat(Svg Eater, Svg Eaten) {
    Eaten.rescale(Point{0.333f, 0.333f});
    Eaten.translate({15.0f, 25.0f});
    Eater.insert(Eaten);
    return Eater;
}


void fail(const char *error) {
    fprintf(stderr, "%s\n", error);
    exit(1);
}