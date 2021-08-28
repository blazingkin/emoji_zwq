#pragma once

#include <cz/str.hpp>
#include <cz/vector.hpp>

namespace xml {

enum Result {
    Success = 0,
    EndOfFile = 1,
    UnclosedTag = -1,
    KeyWithNoValue = -2,
    UnclosedValue = -3,
    ClosingSelfClosingTag = -4,
};

struct Tag_Token {
    enum Type {
        /// <tag>
        Open,
        /// </tag>
        Close,
        /// <tag/>
        SelfClose,
    };

    struct Pair {
        cz::Str key;
        /// Note: if no value is provided then `value.buffer == nullptr`.
        cz::Str value;
    };

    Type type;
    cz::Str name;
    cz::Vector<Pair> pairs;
};
struct Token {
    enum {Text, Tag} tag;
    union {
        cz::Str text;
        Tag_Token tag;
    } v;
};

Result next_token(size_t* index, cz::Str string, cz::Allocator allocator, Token* token);

}
