#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "svg.hpp"
#include <cz/string.hpp>

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
    struct _uclist *Next;
} UnicodeList;


typedef struct {

} EmojiImage;

void fail(const char *);

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
        int      StringOffset  = 0;
        while (StringOffset < StringLength) {
            // Switch on the length encoded in the first byte
            char FirstByte = UserString[StringOffset++];
            if ((FirstByte & 0x80) == 0) {
                // Single byte encoding
                // 0b0xxxxxxx
                CodePoints[CodePointNum++] = FirstByte;
                printf("One byte %x\n", FirstByte);
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
    printf("%d code points\n", CodePointNum);

    uint64_t    *Start  = &CodePoints[0];
    uint32_t     Length = 0;
    UnicodeList *Head   = NULL;
    for (int Index = 0; Index < CodePointNum; Index++) {
        if (CodePoints[Index] == ZWJ) {
            if (Length == 0) {
                Start = &CodePoints[Index + 1];
                printf("Empty segment\n");
                continue;
            }
            UnicodeList *OldHead = Head;
            Head = (UnicodeList *)calloc(sizeof(UnicodeList), 1);
            Head->Next = OldHead;
            Head->CodePoints = Start;
            Start = &CodePoints[Index + 1];
        } else {
            Length++;
        }
    }

    if (Length != 0) {
        UnicodeList *OldHead = Head;
        Head = (UnicodeList *)calloc(sizeof(UnicodeList), 1);
        Head->Next = OldHead;
        Head->CodePoints = Start;
    }

    String str = {"twemoji-svg/1f0cf.svg", strlen("twemoji-svg/1f0cf.svg"), strlen("twemoji-svg/1f0cf.svg")};
    ReadSVGFile(str, NULL);

    // Read in the base emoji images



    // Combine them based on whatever the user requested

    // Output the new images

}


void fail(const char *error) {
    fprintf(stderr, "%s\n", error);
    exit(1);
}