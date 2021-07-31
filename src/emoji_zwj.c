#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define MAX_USER_LEN 200


// References
////////////////////////////////////////////////////////////////////////////////
//
// [REF:UTF-8 Encoding]
//    - https://en.wikipedia.org/wiki/UTF-8#Encoding
//
////////////////////////////////////////////////////////////////////////////////

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
        int      CodePointChar = 0;
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

    // Read in the base emoji images

    // Combine them based on whatever the user requested

    // Output the new images

}


void fail(const char *error) {
    fprintf(stderr, "%s\n", error);
    exit(1);
}