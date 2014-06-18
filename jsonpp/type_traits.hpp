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

#ifndef JSON_TYPE_TRAITS_HPP
#define JSON_TYPE_TRAITS_HPP

#include <type_traits>
#include <string>

namespace json {
inline namespace v1 {
template<typename T>
struct identity {
    using type = T;
};

template<typename T>
using Identity = typename identity<T>::type;

template<bool B>
using Bool = std::integral_constant<bool, B>;

template<typename T>
using Not = Bool<!T::value>;

template<typename Condition, typename Then, typename Else>
using If = typename std::conditional<Condition::value, Then, Else>::type;

template<typename... Args>
struct And : Bool<true> {};

template<typename T, typename... Args>
struct And<T, Args...> : If<T, And<Args...>, Bool<false>> {};

template<typename... Args>
struct Or : Bool<false> {};

template<typename T, typename... Args>
struct Or<T, Args...> : If<T, Bool<true>, Or<Args...>> {};

template<typename... Args>
using EnableIf = typename std::enable_if<And<Args...>::value, int>::type;

template<typename... Args>
using DisableIf = typename std::enable_if<Not<And<Args...>>::value, int>::type;

template<typename T>
struct is_bool : std::is_same<T, bool> {};

template<typename T>
struct is_number : And<std::is_arithmetic<T>, Not<is_bool<T>>> {};

using null = decltype(nullptr);

template<typename T>
struct is_null : std::is_same<T, null> {};

class value;

template<typename T>
struct is_value : public std::is_same<T, value> {};

template<typename T, typename U = typename std::decay<T>::type>
struct is_string : public Or<std::is_same<U, std::string>, std::is_same<U, const char*>, std::is_same<U, char*>> {};

enum class type {
    null, string, boolean, number, array, object
};
} // v1
} // json

#endif // JSON_TYPE_TRAITS_HPP
