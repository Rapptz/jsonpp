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

template<typename Value>
inline void parse_string(parser_state& ps, Value& v) {
    const char* end = ps.str + 1;
    if(*end == '\0') {
        throw parser_error("expected string, received EOF instead", ps.line, ps.column);
    }

    while(*end) {
        if(*end == '\\' && *(end + 1) != '\0' && *(end + 1) == '"') {
            end = end + 2;
        }
        else if(*end == '"') {
            break;
        }
        else {
            ++end;
        }
    }

    if(*end == '\0') {
        throw parser_error("unescaped string sequence found", ps.line, ps.column);
    }

    auto&& temp = std::string(ps.str + 1, end);
    ps.column += temp.size() + 1;
    v = temp;
    ++end;
    ps.str = end;
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
