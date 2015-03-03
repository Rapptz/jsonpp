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

template<typename T>
using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

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
struct is_value : std::is_same<T, value> {};

template<typename T, typename U = typename std::decay<T>::type>
struct is_string : Or<std::is_same<U, std::string>, std::is_same<U, const char*>, std::is_same<U, char*>> {};

struct has_to_json_impl {
    template<typename T, typename U = decltype(to_json(std::declval<T>()))>
    static is_value<U> test(int);

    template<typename...>
    static std::false_type test(...);
};

namespace detail {
using std::end;
using std::begin;

// checks if a type has iterator support
struct has_iterators_impl {
    template<typename T, typename B = decltype(begin(std::declval<T>())),
                         typename E = decltype(end(std::declval<T>()))>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};

template<typename T>
struct is_possible_key_type : Or<is_bool<T>, is_number<T>, is_string<T>, is_value<T>> {};

// a trait to try to guess for a vector as close as possible
struct is_array_impl {
    template<typename T, typename U = Unqualified<T>,
                         typename V = typename U::value_type,
                         typename S = decltype(std::declval<U>().shrink_to_fit()),
                         typename R = decltype(std::declval<U>().reserve(0))>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};

// a trait to try to guess for key value pair as close as possible
struct is_object_impl {
    template<typename T, typename U = Unqualified<T>,
                         typename K = typename U::key_type,
                         typename V = typename U::mapped_type,
                         typename C = typename U::key_compare>
    static is_possible_key_type<K> test(int);
    template<typename...>
    static std::false_type test(...);
};
} // detail

template<typename T>
struct has_iterators : decltype(detail::has_iterators_impl::test<T>(0)) {};

template<typename T>
struct is_array : And<Or<decltype(detail::is_array_impl::test<T>(0)), std::is_array<T>>, has_iterators<T>, Not<is_string<T>>> {};

template<typename T>
struct is_object : And<decltype(detail::is_object_impl::test<T>(0)), has_iterators<T>> {};

template<typename T>
struct has_to_json : decltype(has_to_json_impl::test<T>(0)) {};

enum class type {
    null, string, boolean, number, array, object
};
} // v1
} // json

#endif // JSON_TYPE_TRAITS_HPP
