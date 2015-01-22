// The MIT License (MIT)

// Copyright (c) 2014 Rapptz

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef JSONPP_PARSER_HPP
#define JSONPP_PARSER_HPP

#include "error.hpp"
#include "value.hpp"
#include <cstring>
#include <iosfwd>

namespace json {
inline namespace v1 {
namespace detail {
struct parser_state {
    unsigned line;
    unsigned column;
    const char* str;
};

inline bool is_space(char ch) {
    switch(ch) {
    case 0x0D: // carriage return
    case 0x09: // tab
    case 0x0A: // line feed
    case 0x20: // space
        return true;
    default:
        return false;
    }
}

inline void skip_white_space(parser_state& ps) {
    while(*ps.str != '\0' && is_space(*ps.str)) {
        if(*ps.str == 0x0A) {
            ++ps.line;
            ps.column = 0;
        }
        ++ps.str;
        ++ps.column;
    }
}

inline void parse_value(parser_state& ps, value& v);

inline void parse_null(parser_state& ps, value& v) {
    static const char null_str[] = "null";
    if(*ps.str == '\0') {
        throw parser_error("expected null, received EOF instead", ps.line, ps.column);
    }

    if(std::strncmp(ps.str, null_str, sizeof(null_str) - 1) != 0) {
        throw parser_error("expected null not found", ps.line, ps.column);
    }

    v = nullptr;
    ps.str = ps.str + sizeof(null_str) - 1;
    ps.column += sizeof(null_str);
}

inline void parse_number(parser_state& ps, value& v) {
    static const std::string lookup = "0123456789eE+-.";
    const char* begin = ps.str;
    if(*begin == '\0') {
        throw parser_error("expected number, received EOF instead", ps.line, ps.column);
    }

    while(lookup.find(*ps.str) != std::string::npos) {
        ++ps.str;
    }

    double val = 0.0;
    try {
        std::string temp(begin, ps.str);
        val = std::stod(temp);
        ps.column += temp.size() + 1;
    }
    catch(const std::exception& e) {
        throw parser_error("number could not be parsed properly", ps.line, ps.column);
    }

    v = val;
}

inline int get_codepoint(parser_state& ps, const char*& str) {
    int codepoint = 0;
    for(unsigned i = 0; i < 4; ++i) {
        char hex = *str;
        if(hex <= 0x1F) {
            throw parser_error("incomplete codepoint provided", ps.line, ps.column);
        }

        // convert the hex character to its integral representation
        // e.g., 'F' -> 15
        if(hex >= '0' && hex <= '9') {
            hex -= '0';
        }
        else if(hex >= 'A' && hex <= 'F') {
            hex -= 'A' - 0xA;
        }
        else if(hex >= 'a' && hex <= 'f') {
            hex -= 'a' - 0xA;
        }
        else {
            throw parser_error("invalid codepoint provided", ps.line, ps.column);
        }

        codepoint = codepoint * 16 + hex;
        ++ps.column;
        ++str;
    }
    return codepoint;
}

inline void parse_codepoint(parser_state& ps, const char*& str, std::string& result) {
    // parse the hex characters
    // in order to do so, we have to increment by one to get these digits.
    // ergo, *str == 'u', ++str = codepoint
    ++str;
    ++ps.column;
    int codepoint = get_codepoint(ps, str);

    // a regular ASCII code point
    if(codepoint < 0x80) {
        result.push_back(codepoint);
        return;
    }

    // handle surrogate pairs
    if(codepoint >= 0xD800 && codepoint <= 0xDFFF) {
        if(codepoint >= 0xDC00) {
            throw parser_error("low surrogate pair found but high surrogate pair expected", ps.line, ps.column);
        }

        // get the  low surrogate pair
        if(*(str + 1) != '\\' && *(str + 2) != 'u') {
            throw parser_error("low surrogate pair expected but not found", ps.line, ps.column);
        }

        str += 2;
        int low_surrogate = get_codepoint(ps, str);
        if(low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
            throw parser_error("low surrogate out of range [\\uDC000, \\uDFFF]", ps.line, ps.column);
        }

        codepoint = 0x10000 + (((codepoint - 0xD800) << 10) | ((low_surrogate - 0xDC00) & 0x3FF));
    }

    if(codepoint < 0x800) {
        result.push_back(0xC0 | (codepoint >> 6));
    }
    else if(codepoint < 0x10000) {
        result.push_back(0xE0 | (codepoint >> 12));
    }
    else {
        result.push_back(0xF0 | (codepoint >> 18));
        result.push_back(0x80 | ((codepoint >> 12) & 0x3F));
    }
    result.push_back(0x80 | ((codepoint >> 6) & 0x3F));
    result.push_back(0x80 | (codepoint & 0x3F));
}

template<typename Value>
inline void parse_string(parser_state& ps, Value& v) {
    const char* str = ps.str + 1;
    if(*str == '\0') {
        throw parser_error("expected string, received EOF instead", ps.line, ps.column);
    }

    std::string result;

    // begin parsing
    while(true) {
        ++ps.column;
        bool increment_string = true;

        if(*str <= 0x1F) {
            throw parser_error("invalid characters found in string or string is incomplete", ps.line, ps.column);
        }

        // end of string found
        if(*str == '"') {
            break;
        }

        // non-escape character is good to go
        if(*str != '\\') {
            result.push_back(*str++);
            continue;
        }

        // at this point *str == '\\'
        // so increment it to check the next character
        ++str;
        switch(*str) {
        case '/':
            result.push_back('/');
            break;
        case '\\':
            result.push_back('\\');
            break;
        case '"':
            result.push_back('\"');
            break;
        case 'f':
            result.push_back('\f');
            break;
        case 'n':
            result.push_back('\n');
            break;
        case 'r':
            result.push_back('\r');
            break;
        case 't':
            result.push_back('\t');
            break;
        case 'b':
            result.push_back('\b');
            break;
        case 'u':
            parse_codepoint(ps, str, result);
            increment_string = false;
            break;
        default:
            throw parser_error("improper or incomplete escape character found", ps.line, ps.column);
        }

        if(increment_string) {
            ++str;
        }
    }

    v = result;
    ++str;
    ps.str = str;
}

inline void parse_bool(parser_state& ps, value& v) {
    if(*ps.str == '\0') {
        throw parser_error("expected boolean, received EOF instead", ps.line, ps.column);
    }

    bool expected_true = *ps.str == 't';
    const char* boolean = expected_true ? "true" : "false";
    const size_t len = expected_true ? 4 : 5;

    if(std::strncmp(ps.str, boolean, len) != 0) {
        throw parser_error("expected boolean not found", ps.line, ps.column);
    }

    v = expected_true;
    ps.str = ps.str + len;
    ps.column += len;
}

inline void parse_array(parser_state& ps, value& v) {
    ++ps.str;
    array arr;
    value elem;
    skip_white_space(ps);

    if(*ps.str == '\0') {
        throw parser_error("expected value, received EOF instead", ps.line, ps.column);
    }

    while (*ps.str && *ps.str != ']') {
        parse_value(ps, elem);
        if(*ps.str != ',') {
            if(*ps.str != ']') {
                throw parser_error("missing comma", ps.line, ps.column);
            }
        }
        else if(*ps.str == ',') {
            ++ps.str;
            // skip whitespace
            skip_white_space(ps);
            // handle missing input
            if(*ps.str && *ps.str == ']') {
                throw parser_error("extraneous comma spotted", ps.line, ps.column);
            }
        }

        arr.push_back(elem);
    }

    v = arr;
    if(*ps.str == ']') {
        ++ps.str;
    }
}

inline void parse_object(parser_state& ps, value& v) {
    ++ps.str;
    object obj;
    std::string key;
    value elem;

    skip_white_space(ps);

    if(*ps.str == '\0') {
        throw parser_error("expected string key, received EOF instead", ps.line, ps.column);
    }

    while(*ps.str) {
        skip_white_space(ps);

        // empty object
        if(*ps.str == '}') {
            break;
        }

        if(*ps.str != '"') {
            throw parser_error("expected string as key not found", ps.line, ps.column);
        }
        parse_string(ps, key);
        skip_white_space(ps);

        if(*ps.str != ':') {
            throw parser_error("missing semicolon", ps.line, ps.column);
        }
        ++ps.str;
        parse_value(ps, elem);

        if(*ps.str != ',') {
            if(*ps.str != '}') {
                throw parser_error("missing comma", ps.line, ps.column);
            }
        }
        else if(*ps.str == ',') {
            ++ps.str;
        }
        obj.emplace(key, elem);
    }

    v = obj;
    if(*ps.str == '}') {
        ++ps.str;
    }
}

inline void parse_value(parser_state& ps, value& v) {
    skip_white_space(ps);
    if(*ps.str == '\0') {
        throw parser_error("unexpected EOF found", ps.line, ps.column);
    }

    if(isdigit(*ps.str) || *ps.str == '+' || *ps.str == '-') {
        parse_number(ps, v);
    }
    else {
        switch(*ps.str) {
        case 'n':
            parse_null(ps, v);
            break;
        case '"':
            parse_string(ps, v);
            break;
        case 't':
        case 'f':
            parse_bool(ps, v);
            break;
        case '[':
            parse_array(ps, v);
            break;
        case '{':
            parse_object(ps, v);
            break;
        default:
            throw parser_error("unexpected token found", ps.line, ps.column);
            break;
        }
    }

    skip_white_space(ps);
}
} // detail

inline void parse(const std::string& str, value& v) {
    detail::parser_state ps = { 1, 1, str.c_str() };
    detail::parse_value(ps, v);
    if(*ps.str != '\0') {
        throw parser_error("unexpected token found", ps.line, ps.column);
    }
}

template<typename IStream, DisableIf<is_string<IStream>> = 0>
inline void parse(IStream& in, value& v) {
    static_assert(std::is_base_of<std::istream, IStream>::value, "Input stream passed must inherit from std::istream");
    if(in) {
        std::ostringstream ss;
        ss << in.rdbuf();
        parse(ss.str(), v);
    }
}
} // v1
} // json

#endif // JSONPP_PARSER_HPP
