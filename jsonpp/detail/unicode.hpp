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

#ifndef JSONPP_DETAIL_UNICODE_HPP
#define JSONPP_DETAIL_UNICODE_HPP

#if defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN 0xFEEDC0DE
#endif // WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX)
#define NOMINMAX 0xFEEDC0DE
#endif // NOMINMAX
#include <windows.h>
#endif
#include <string>
#include <stdexcept>
#include <system_error>
#include <cerrno>

namespace json {
inline namespace v1 {
namespace detail {
#if defined(_WIN32)
inline std::u16string utf8_to_utf16(const std::string& utf8) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t must be 16 bits");
    std::wstring temp;
    auto required_size = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], static_cast<int>(utf8.size()), nullptr, 0);
    temp.resize(required_size);
    if(::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], static_cast<int>(utf8.size()), &temp[0], required_size) == 0) {
        throw std::runtime_error("unable to convert from UTF-8 to UTF-16");
    }
    return { temp.begin(), temp.end() };
}
#else
inline std::u16string utf8_to_utf16(const std::string& utf8) {
    std::u16string result;
    using size_type = decltype(utf8.size());
    std::invalid_argument invalid_utf8("Invalid UTF-8 string given");
    size_type i = 0;
    while(i < utf8.size()) {
        char32_t codepoint;
        size_type iterations = 0;
        unsigned char byte = utf8[i++];
        if(byte <= 0x7F) {
            codepoint = byte;
        }
        else if(byte <= 0xBF) {
            throw invalid_utf8;
        }
        else if(byte <= 0xDF) {
            codepoint = byte & 0x1F;
            iterations = 1;
        }
        else if(byte <= 0xEF) {
            codepoint = byte & 0x0F;
            iterations = 2;
        }
        else if(byte <= 0xF7) {
            codepoint = byte & 0x07;
            iterations = 3;
        }
        else {
            throw invalid_utf8;
        }

        for(size_type j = 0; j < iterations; ++j) {
            if(i == utf8.size()) {
                throw invalid_utf8;
            }
            unsigned char next_byte = utf8[i++];
            if(next_byte < 0x80 || next_byte > 0xBF) {
                throw invalid_utf8;
            }

            codepoint = (codepoint << 6) + (next_byte & 0x3F);
        }
        if(codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
            throw invalid_utf8;
        }

        if(codepoint <= 0xFFFF) {
            result.push_back(codepoint);
        }
        else {
            codepoint -= 0x10000;
            result.push_back((codepoint >> 10) + 0xD800);
            result.push_back((codepoint & 0x3FF) + 0xDC00);
        }
    }

    return result;
}
#endif
} // detail
} // v1
} // json

#if defined(WIN32_LEAN_AND_MEAN) && WIN32_LEAN_AND_MEAN == 0xFEEDC0DE
#undef WIN32_LEAN_AND_MEAN
#endif

#if defined(NOMINMAX) && NOMINMAX == 0xFEEDC0DE
#undef NOMINMAX
#endif

#endif // JSONPP_DETAIL_UNICODE_HPP
