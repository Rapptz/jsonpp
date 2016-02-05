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

#ifndef JSONPP_TYPE_NAME_HPP
#define JSONPP_TYPE_NAME_HPP

#include "type_traits.hpp"

namespace json {
template<typename T, typename Sfinae = int>
struct type_name;

template<typename T>
struct type_name<T, EnableIf<is_null<T>>> {
    static constexpr const char* value = "null";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_null<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_bool<T>>> {
    static constexpr const char* value = "boolean";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_bool<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_number<T>>> {
    static constexpr const char* value = "number";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_number<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_string<T>>> {
    static constexpr const char* value = "string";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_string<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_object<T>>> {
    static constexpr const char* value = "object";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_object<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_array<T>>> {
    static constexpr const char* value = "array";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_array<T>>>::value;
} // json

#endif // JSONPP_TYPE_NAME_HPP
