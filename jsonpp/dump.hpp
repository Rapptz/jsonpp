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

#ifndef JSONPP_DUMP_HPP
#define JSONPP_DUMP_HPP

#include "type_traits.hpp"
#include "detail/unicode.hpp"
#include <string>
#include <sstream>
#include <iosfwd>
#include <cmath>

namespace json {
inline namespace v1 {
struct format_options {
    enum : int {
        none = 0,
        allow_nan_inf = 1 << 0,
        minify = 1 << 1,
        escape_multi_byte = 1 << 2
    };

    int flags = none;
    int indent = 4;
    int depth = 0;
};

namespace detail {
template<typename OStream>
inline void indent(OStream& out, const format_options& opt) {
    out << '\n';
    for(int i = 0; i < (opt.indent * opt.depth); ++i) {
        out << ' ';
    }
}
} // detail


template<typename OStream, typename T, EnableIf<is_null<T>> = 0>
inline OStream& dump(OStream& out, const T&, format_options = {}) {
    out << "null";
    return out;
}

template<typename OStream, typename T, EnableIf<is_bool<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options = {}) {
    out << (t ? "true" : "false");
    return out;
}

template<typename OStream, typename T, EnableIf<is_number<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    if((opt.flags & opt.allow_nan_inf) != opt.allow_nan_inf && (std::isnan(t) || std::isinf(t))) {
        // stream null instead if nan is found
        out << "null";
        return out;
    }
    auto precision = out.precision();
    out.precision(17);
    out << t;
    out.precision(precision);
    return out;
}

namespace detail {
template<typename OStream>
inline bool escape_control(OStream& out, char control) {
    switch(control) {
    case '"':
        out << "\\\"";
        return true;
    case '\\':
        out << "\\\\";
        return true;
    case '/':
        out << "\\/";
        return true;
    case '\b':
        out << "\\b";
        return true;
    case '\f':
        out << "\\f";
        return true;
    case '\n':
        out << "\\n";
        return true;
    case '\r':
        out << "\\r";
        return true;
    case '\t':
        out << "\\t";
        return true;
    default:
        return false;
    }
}

template<typename OStream>
inline OStream& escape_str(OStream& out, const std::u16string& utf16) {
    auto fill = out.fill();
    auto width = out.width();
    auto flags = out.flags();
    out << '"';
    for(auto&& c : utf16) {
        if(c <= 0x7F) {
            if(escape_control(out, static_cast<char>(c))) {
                continue;
            }
            out << static_cast<char>(c);
        }
        else {
            // prepare stream
            out << "\\u";
            out.width(4);
            out.fill('0');
            out.setf(out.hex, out.basefield);
            out << c;
            out.width(width);
            out.fill(fill);
            out.flags(flags);
        }
    }
    out << '"';
    return out;
}
} // detail

template<typename OStream, typename T, EnableIf<is_string<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    bool escape = (opt.flags & opt.escape_multi_byte) == opt.escape_multi_byte;
    if(escape) {
        return detail::escape_str(out, detail::utf8_to_utf16(t));
    }
    out << '"';
    for(auto&& c : t) {
        if(detail::escape_control(out, c)) {
            continue;
        }
        else {
            out << c;
        }
    }
    out << '"';
    return out;
}

template<typename OStream, typename T, EnableIf<is_array<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    bool prettify = (opt.flags & opt.minify) != opt.minify;
    opt.depth += prettify;
    out << '[';

    using std::begin;
    using std::end;
    auto&& first = begin(t);
    auto&& last  = end(t);
    bool first_pass = true;
    for(; first != last; ++first) {
        if(not first_pass) {
            out << ',';
        }

        if(prettify) {
            detail::indent(out, opt);
        }

        dump(out, *first, opt);
        first_pass = false;
    }

    if(prettify) {
        --opt.depth;
        if(not first_pass) {
            detail::indent(out, opt);
        }
    }

    out << ']';
    return out;
}

namespace detail {
template<typename OStream, typename T, EnableIf<std::is_arithmetic<T>> = 0>
inline void key(OStream& out, const T& t, const format_options&) {
    out << '"' << std::to_string(t) << '"';
}

template<typename OStream, typename T, DisableIf<std::is_arithmetic<T>> = 0>
inline void key(OStream& out, const T& t, const format_options& opt) {
    dump(out, t, opt);
}
} // detail

template<typename OStream, typename T, EnableIf<is_object<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    bool prettify = (opt.flags & format_options::minify) != format_options::minify;
    opt.depth += prettify;
    out << '{';

    using std::begin;
    using std::endl;

    auto&& first = begin(t);
    auto&& last  = end(t);
    bool first_pass = true;

    for(; first != last; ++first) {

        auto&& elem = *first;
        if(not first_pass) {
            out << ',';
        }

        if(prettify) {
            detail::indent(out, opt);
        }

        detail::key(out, elem.first, opt);
        out << ':';

        if(prettify) {
            out << ' ';
        }

        dump(out, elem.second, opt);
        first_pass = false;
    }

    if(prettify) {
        --opt.depth;

        if(not first_pass) {
            detail::indent(out, opt);
        }
    }

    out << '}';
    return out;
}
} // v1
} // json

#endif // JSONPP_DUMP_HPP
