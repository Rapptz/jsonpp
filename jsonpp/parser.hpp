// The MIT License (MIT)

// Copyright (c) 2014-2016 Danny Y.

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

struct extensions {
    enum : unsigned {
        none = 0,
        comments = 1 << 1,
        trailing_comma = 1 << 2,
        all = comments | trailing_comma
    };
};

template<unsigned Flags>
struct parser {
private:
    std::size_t line = 1;
    std::size_t column = 1;
    const char* str;

    template<unsigned X = Flags, EnableIf<has_extension<X, extensions::comments>> = 0>
    void skip_comments() {
        if(*str != '/') {
            return;
        }
        const char* copy = str + 1;
        switch(*copy) {
        case '/':
            for(++copy; *copy != '\0' && *copy != 0x0A; ++copy) {
                  ++column;
            }
            break;
        case '*':
            ++copy;
            if (*copy == '\0')
                return;
            for(const char* prev = copy++; true; ++prev, ++copy) {
                if (*copy == '/' && *prev == '*') {
                    ++copy;
                    break;
                }
                if (*copy == '\0')
                    throw parser_error("expected */, received EOF instead", line, column);
                if(*str == 0x0A) {
                    ++line;
                    column = 0;
                }
                ++column;
            }
            break;
        case '\0':
        default:
            return;
        }
        str = copy;
    }

    template<unsigned X = Flags, DisableIf<has_extension<X, extensions::comments>> = 0>
    void skip_comments() {}

    void skip_white_space() {
        while(*str != '\0') {
            skip_comments();
            if (!is_space(*str)) {
                return;
            }
            if(*str == 0x0A) {
                ++line;
                column = 0;
            }
            ++str;
            ++column;
        }
    }

    template<unsigned X = Flags, EnableIf<has_extension<X, extensions::trailing_comma>> = 0>
    void check_trailing_comma(bool) {}

    template<unsigned X = Flags, DisableIf<has_extension<X, extensions::trailing_comma>> = 0>
    void check_trailing_comma(bool has_trailing_comma) {
        if(has_trailing_comma) {
            throw parser_error("extraneous comma found", line, column);
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

        if(*str == '-') {
            ++str; // started with -
        }

        if(*str == '0') {
            // starting with 0
            ++str;
            if(*str >= '0' && *str <= '9') {
                // leading zero
                auto offset = column + (str - begin);
                throw parser_error("numbers cannot start with a zero", line, offset);
            }
        }

        while(lookup.find(*str) != std::string::npos) {
            ++str;
        }

        auto size = static_cast<size_t>(str - begin);
        double val = 0.0;
        size_t idx;
        try {
            std::string temp(begin, str);
            val = std::stod(temp, &idx);
            column += temp.size() + 1;
        }
        catch(const std::exception&) {
            throw parser_error("number could not be parsed properly", line, column);
        }

        if(idx != size) {
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
            if(*copy != '\\' && *(copy + 1) != 'u') {
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

        // give an initial buffer of 64 to allow constant reallocation
        result.reserve(64);

        // begin parsing
        while(true) {
            ++column;
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
                ++copy;
                break;
            case '\\':
                result.push_back('\\');
                ++copy;
                break;
            case '"':
                result.push_back('\"');
                ++copy;
                break;
            case 'f':
                result.push_back('\f');
                ++copy;
                break;
            case 'n':
                result.push_back('\n');
                ++copy;
                break;
            case 'r':
                result.push_back('\r');
                ++copy;
                break;
            case 't':
                result.push_back('\t');
                ++copy;
                break;
            case 'b':
                result.push_back('\b');
                ++copy;
                break;
            case 'u':
                parse_codepoint(copy, result);
                break;
            default:
                throw parser_error("improper or incomplete escape character found", line, column);
            }
        }

        result.shrink_to_fit();
        v = std::move(result);
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
        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected value, received EOF instead", line, column);
        }

        while (*str && *str != ']') {
            value elem;
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
                bool is_trailing_comma = *str && *str == ']';
                check_trailing_comma(is_trailing_comma);
            }

            arr.push_back(std::move(elem));
        }

        v = std::move(arr);
        if(*str == ']') {
            ++str;
        }
    }

    void parse_object(value& v) {
        ++str;
        object obj;
        bool last_is_comma = false;

        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected string key, received EOF instead", line, column);
        }

        while(*str) {
            std::string key;
            value elem;

            skip_white_space();

            // empty object
            if(*str == '}') {
                check_trailing_comma(last_is_comma);
                break;
            }

            last_is_comma = false;

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
                last_is_comma = true;
                ++column;
                ++str;
            }
            obj.emplace(std::move(key), std::move(elem));
        }

        v = std::move(obj);
        if(*str == '}') {
            ++str;
            ++column;
        }
        else {
            throw parser_error("expected closing brace", line, column);
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
    parser(const char* str) JSONPP_NOEXCEPT: str(str) {}

    void parse(value& v) {
        parse_value(v);
        if(*str != '\0') {
            throw parser_error("unexpected token found", line, column);
        }
    }
};

template<unsigned Flags = extensions::none>
inline void parse(const std::string& str, value& v) {
    parser<Flags> js(str.c_str());
    js.parse(v);
}

template<unsigned Flags = extensions::none, typename IStream, DisableIf<is_string<IStream>> = 0>
inline void parse(IStream& in, value& v) {
    static_assert(std::is_base_of<std::istream, IStream>::value, "Input stream passed must inherit from std::istream");
    if(in) {
        std::ostringstream ss;
        ss << in.rdbuf();
        parse<Flags>(ss.str(), v);
    }
}

template<unsigned Flags = extensions::none, typename T>
inline value parse(T&& t) {
    value v;
    parse<Flags>(std::forward<T>(t), v);
    return v;
}
} // json

#endif // JSONPP_PARSER_HPP
