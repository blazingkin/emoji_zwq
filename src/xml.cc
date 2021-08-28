#include "xml.hpp"

#include <cz/char_type.hpp>
#include <cz/defer.hpp>

namespace xml {

static void skip_spaces(size_t* index, cz::Str string);
static Result parse_tag_token(size_t* index, cz::Str string, cz::Allocator allocator, Token* token);
static Result parse_text_token(size_t* index, cz::Str string, Token* token);
static Result parse_value(size_t* index, cz::Str string, cz::Str* value);

Result next_token(size_t* index, cz::Str string, cz::Allocator allocator, Token* token) {
    skip_spaces(index, string);
    if (*index >= string.len)
        return EndOfFile;

    if (string[*index] == '<') {
        return parse_tag_token(index, string, allocator, token);
    } else {
        return parse_text_token(index, string, token);
    }
}

static Result parse_tag_token(size_t* index,
                              cz::Str string,
                              cz::Allocator allocator,
                              Token* token) {
    CZ_DEBUG_ASSERT(string[*index] == '<');
    ++*index;

    if (*index >= string.len)
        return UnclosedTag;

    Tag_Token::Type type = Tag_Token::Open;
    if (string[*index] == '/') {
        type = Tag_Token::Close;
        ++*index;
    }

    skip_spaces(index, string);

    size_t name_start = *index;
    while (*index < string.len && cz::is_alnum(string[*index]))
        ++*index;
    cz::Str name = string.slice(name_start, *index);

    cz::Vector<Tag_Token::Pair> pairs = {};
    CZ_DEFER(pairs.drop(allocator));

    // Parse pairs.
    while (1) {
        skip_spaces(index, string);
        if (*index >= string.len)
            return UnclosedTag;

        if (string.slice_start(*index).starts_with("/>")) {
            *index += 2;
            if (type == Tag_Token::Close)
                return ClosingSelfClosingTag;
            type = Tag_Token::SelfClose;
            break;
        }
        if (string[*index] == '>') {
            ++*index;
            break;
        }

        size_t key_start = *index;
        while (1) {
            if (*index >= string.len)
                return UnclosedTag;
            if (cz::is_space(string[*index]) || string[*index] == '=')
                break;
            ++*index;
        }
        cz::Str key = string.slice(key_start, *index);

        cz::Str value;
        Result value_result = parse_value(index, string, &value);
        if (value_result != Success)
            return value_result;

        pairs.reserve(allocator, 1);
        pairs.push({key, value});
    }

    token->tag = Token::Tag;
    token->v.tag.type = type;
    token->v.tag.name = name;
    token->v.tag.pairs = pairs;
    pairs = {};
    return Success;
}

static Result parse_value(size_t* index, cz::Str string, cz::Str* value) {
    if (string[*index] != '=') {
        *value = {};
        CZ_DEBUG_ASSERT(value->buffer == nullptr);
        return Success;
    }
    ++*index;

    if (*index >= string.len)
        return UnclosedTag;
    if (string[*index] != '"' && string[*index] != '\'')
        return KeyWithNoValue;

    ++*index;
    size_t value_start = *index;
    while (true) {
        if (*index >= string.len)
            return UnclosedValue;
        if (string[*index] == string[value_start - 1] && string[*index - 1] != '\\')
            break;
        ++*index;
    }

    *value = string.slice(value_start, *index);
    CZ_DEBUG_ASSERT(value->buffer != nullptr);

    CZ_DEBUG_ASSERT(string[*index] == string[value_start - 1]);
    ++*index;
    return Success;
}

static Result parse_text_token(size_t* index, cz::Str string, Token* token) {
    const char* endp = string.find('<');
    size_t end = endp ? endp - string.buffer : string.len;

    // We don't need a bounds check because we know there
    // is a non-space character at the start of our token.
    while (cz::is_space(string[end - 1])) {
        --end;
    }

    token->tag = Token::Text;
    token->v.text = string.slice(*index, end);

    *index = end;
    return Success;
}

static void skip_spaces(size_t* index, cz::Str string) {
    while (*index < string.len && cz::is_space(string[*index]))
        ++*index;
}

}
