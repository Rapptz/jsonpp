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

#ifndef JSONPP_TO_JSON_KEY_HPP
#define JSONPP_TO_JSON_KEY_HPP

#include "type_traits.hpp"
#include "overload_resolution.hpp"
#include <string>

namespace json {
namespace detail {
struct to_json_key_impl {
private:
    template<typename T, typename U = Unqualified<T>, EnableIf<has_to_json_key<U>> = 0>
    auto impl(choice<0>, T&& value) const -> decltype(to_json_key(std::forward<T>(value))) {
        return to_json_key(std::forward<T>(value));
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_string<U>> = 0>
    auto impl(choice<1>, T&& value) const -> decltype(std::forward<T>(value)) {
        return std::forward<T>(value);
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_bool<U>> = 0>
    std::string impl(choice<2>, T&& value) const {
        return std::forward<T>(value) ? "true" : "false";
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<std::is_same<U, char>> = 0>
    std::string impl(choice<3>, T&& value) const {
        return std::string(1, std::forward<T>(value));
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<std::is_arithmetic<U>> = 0>
    auto impl(choice<4>, T&& value) const -> decltype(std::to_string(std::forward<T>(value))) {
        return std::to_string(std::forward<T>(value));
    }
public:
    template<typename T>
    auto operator()(T&& value) const -> decltype(impl(select_overload{}, std::forward<T>(value))) {
        return impl(select_overload{}, std::forward<T>(value));
    }
};
} // detail

constexpr detail::to_json_key_impl to_json_key{};
} // json

#endif // JSONPP_TO_JSON_KEY_HPP
