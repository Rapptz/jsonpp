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
struct parser {
private:
    unsigned line = 1;
    unsigned column = 1;

    const char* skip_white_space(const char* str) {
        while(*str && isspace(*str)) {
            if(*str == '\n') {
                ++line;
                column = 0;
            }
            ++str;
            ++column;
        }
        return str;
    }

    const char* parse_value(const char* str, value& v) {
        const char* s = skip_white_space(str);
        if(*s == '\0') {
            throw parser_error("unexpected EOF found", line, column);
        }

        if(isdigit(*s) || *s == '+' || *s == '-') {
            s = parse_number(s, v);
        }
        else {
            switch(*s) {
            case 'n':
                s = parse_null(s, v);
                break;
            case '"':
                s = parse_string(s, v);
                break;
            case 't':
            case 'f':
                s = parse_bool(s, v);
                break;
            case '[':
                s = parse_array(s, v);
                break;
            case '{':
                s = parse_object(s, v);
                break;
            default:
                throw parser_error("unexpected token found", line, column);
                break;
            }
        }

        return skip_white_space(s);
    }

    const char* parse_null(const char* str, value& v) {
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
        return str;
    }

    const char* parse_number(const char* str, value& v) {
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
        return str;
    }

    template<typename Value>
    const char* parse_string(const char* str, Value& v) {
        const char* end = str + 1;
        if(*end == '\0') {
            throw parser_error("expected string, received EOF instead", line, column);
        }

        while(*end && *end != '"') {
            ++end;
        }

        if(*end == '\0') {
            throw parser_error("unescaped string sequence found", line, column);
        }

        auto&& temp = std::string(str + 1, end);
        column += temp.size() + 1;
        v = temp;
        ++end;
        return end;
    }

    const char* parse_bool(const char* str, value& v) {
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
        return str;
    }

    const char* parse_array(const char* str, value& v) {
        const char *s = str + 1;
        array arr;
        value elem;
        s = skip_white_space(s);

        if(*s == '\0') {
            throw parser_error("expected value, received EOF instead", line, column);
        }

        while (*s && *s != ']') {
            s = parse_value(s, elem);
            if(*s != ',') {
                if(*s != ']') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*s == ',') {
                ++s;
                // skip whitespace
                s = skip_white_space(s);
                // handle missing input
                if(*s && *s == ']') {
                    throw parser_error("extraneous comma spotted", line, column);
                }
            }

            arr.push_back(elem);
        }
        v = arr;
        return *s == ']' ? s + 1 : s;
    }

    const char* parse_object(const char* str, value& v) {
        const char* s = str + 1;
        object obj;
        std::string key;
        value elem;

        s = skip_white_space(s);

        if(*s == '\0') {
            throw parser_error("expected string key, received EOF instead", line, column);
        }

        while(*s) {
            s = skip_white_space(s);

            // empty object
            if(*s == '}') {
                break;
            }

            if(*s != '"') {
                throw parser_error("expected string as key not found", line, column);
            }
            s = parse_string(s, key);
            s = skip_white_space(s);

            if(*s != ':') {
                throw parser_error("missing semicolon", line, column);
            }
            ++s;
            s = parse_value(s, elem);

            if(*s != ',') {
                if(*s != '}') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*s == ',') {
                ++s;
            }
            obj.emplace(key, elem);
        }

        v = obj;
        return *s == '}' ? s + 1 : s;
    }
public:
    parser() = default;
    parser(const std::string& str, value& v) {
        parse(str, v);
    }

    template<typename IStream, DisableIf<is_string<IStream>> = 0>
    parser(IStream& in, value& v) {
        parse(in, v);
    }

    void parse(const std::string& str, value& v) {
        line = 1;
        column = 1;
        auto s = parse_value(str.c_str(), v);
        if(*s != '\0') {
            throw parser_error("unexpected token found", line, column);
        }
    }

    template<typename IStream, DisableIf<is_string<IStream>> = 0>
    void parse(IStream& in, value& v) {
        static_assert(std::is_base_of<std::istream, IStream>::value, "Input stream passed must inherit from std::istream");
        if(in) {
            in.seekg(0, in.end);
            auto size = in.tellg();
            in.seekg(0, in.beg);
            std::string str;
            str.resize(size);
            in.read(&str[0], size);
            parse(str, v);
        }
    }
};
} // v1
} // json

#endif // JSONPP_PARSER_HPP
