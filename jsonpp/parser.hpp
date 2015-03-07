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

struct parser {
private:
    unsigned line = 1;
    unsigned column = 1;
    const char* str;

    void skip_white_space() {
        while(*str != '\0' && is_space(*str)) {
            if(*str == 0x0A) {
                ++line;
                column = 0;
            }
            ++str;
            ++column;
        }
    }

    void parse_null(value& v) {
        static const char null_str[] = "null";
        if(*str == '\0') {
            throw parser_error("expected null, received EOF instead", line, column);
        }

        if(std::strncmp(str, null_str, sizeof(null_str) - 1) != 0) {
            throw parser_error("expected null not found", line, column);
        }

        v = nullptr;
        str = str + sizeof(null_str) - 1;
        column += sizeof(null_str);
    }

    void parse_number(value& v) {
        static const std::string lookup = "0123456789eE+-.";
        const char* begin = str;
        if(*begin == '\0') {
            throw parser_error("expected number, received EOF instead", line, column);
        }

        while(lookup.find(*str) != std::string::npos) {
            ++str;
        }

        double val = 0.0;
        try {
            std::string temp(begin, str);
            val = std::stod(temp);
            column += temp.size() + 1;
        }
        catch(const std::exception& e) {
            throw parser_error("number could not be parsed properly", line, column);
        }

        v = val;
    }

    int get_codepoint(const char*& copy) {
        int codepoint = 0;
        for(unsigned i = 0; i < 4; ++i) {
            char hex = *copy;
            if(hex <= 0x1F) {
                throw parser_error("incomplete codepoint provided", line, column);
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
                throw parser_error("invalid codepoint provided", line, column);
            }

            codepoint = codepoint * 16 + hex;
            ++column;
            ++copy;
        }
        return codepoint;
    }

    void parse_codepoint(const char*& copy, std::string& result) {
        // parse the hex characters
        // in order to do so, we have to increment by one to get these digits.
        // ergo, *copy == 'u', ++copy = codepoint
        ++copy;
        ++column;
        int codepoint = get_codepoint(copy);

        // a regular ASCII code point
        if(codepoint < 0x80) {
            result.push_back(codepoint);
            return;
        }

        // handle surrogate pairs
        if(codepoint >= 0xD800 && codepoint <= 0xDFFF) {
            if(codepoint >= 0xDC00) {
                throw parser_error("low surrogate pair found but high surrogate pair expected", line, column);
            }

            // get the  low surrogate pair
            if(*(copy + 1) != '\\' && *(copy + 2) != 'u') {
                throw parser_error("low surrogate pair expected but not found", line, column);
            }

            copy += 2;
            int low_surrogate = get_codepoint(copy);
            if(low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                throw parser_error("low surrogate out of range [\\uDC000, \\uDFFF]", line, column);
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
    void parse_string(Value& v) {
        const char* copy = str + 1;
        if(*copy == '\0') {
            throw parser_error("expected string, received EOF instead", line, column);
        }

        std::string result;

        // begin parsing
        while(true) {
            ++column;
            bool increment_string = true;
            auto byte = static_cast<unsigned char>(*copy);

            if(byte <= 0x1F) {
                throw parser_error("invalid characters found in string or string is incomplete", line, column);
            }

            // end of string found
            if(*copy == '"') {
                break;
            }

            // non-escape character is good to go
            if(*copy != '\\') {
                result.push_back(*copy++);
                continue;
            }

            // at this point *copy == '\\'
            // so increment it to check the next character
            ++copy;
            switch(*copy) {
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
                parse_codepoint(copy, result);
                increment_string = false;
                break;
            default:
                throw parser_error("improper or incomplete escape character found", line, column);
            }

            if(increment_string) {
                ++copy;
            }
        }

        v = result;
        ++copy;
        str = copy;
    }

    void parse_bool(value& v) {
        if(*str == '\0') {
            throw parser_error("expected boolean, received EOF instead", line, column);
        }

        bool expected_true = *str == 't';
        const char* boolean = expected_true ? "true" : "false";
        const size_t len = expected_true ? 4 : 5;

        if(std::strncmp(str, boolean, len) != 0) {
            throw parser_error("expected boolean not found", line, column);
        }

        v = expected_true;
        str = str + len;
        column += len;
    }

    void parse_array(value& v) {
        ++str;
        array arr;
        value elem;
        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected value, received EOF instead", line, column);
        }

        while (*str && *str != ']') {
            parse_value(elem);
            if(*str != ',') {
                if(*str != ']') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*str == ',') {
                ++str;
                // skip whitespace
                skip_white_space();
                // handle missing input
                if(*str && *str == ']') {
                    throw parser_error("extraneous comma spotted", line, column);
                }
            }

            arr.push_back(elem);
        }

        v = arr;
        if(*str == ']') {
            ++str;
        }
    }

    void parse_object(value& v) {
        ++str;
        object obj;
        std::string key;
        value elem;

        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected string key, received EOF instead", line, column);
        }

        while(*str) {
            skip_white_space();

            // empty object
            if(*str == '}') {
                break;
            }

            if(*str != '"') {
                throw parser_error("expected string as key not found", line, column);
            }
            parse_string(key);
            skip_white_space();

            if(*str != ':') {
                throw parser_error("missing semicolon", line, column);
            }
            ++str;
            parse_value(elem);

            if(*str != ',') {
                if(*str != '}') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*str == ',') {
                ++str;
            }
            obj.emplace(key, elem);
        }

        v = obj;
        if(*str == '}') {
            ++str;
        }
    }

    void parse_value(value& v) {
        skip_white_space();
        if(*str == '\0') {
            throw parser_error("unexpected EOF found", line, column);
        }

        if(isdigit(*str) || *str == '+' || *str == '-') {
            parse_number(v);
        }
        else {
            switch(*str) {
            case 'n':
                parse_null(v);
                break;
            case '"':
                parse_string(v);
                break;
            case 't':
            case 'f':
                parse_bool(v);
                break;
            case '[':
                parse_array(v);
                break;
            case '{':
                parse_object(v);
                break;
            default:
                throw parser_error("unexpected token found", line, column);
                break;
            }
        }

        skip_white_space();
    }
public:
    parser(const char* str) noexcept: str(str) {}

    void parse(value& v) {
        parse_value(v);
        if(*str != '\0') {
            throw parser_error("unexpected token found", line, column);
        }
    }
};

inline void parse(const std::string& str, value& v) {
    parser js(str.c_str());
    js.parse(v);
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
