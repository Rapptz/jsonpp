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

#ifndef JSONPP_CONVERT_HPP
#define JSONPP_CONVERT_HPP

#include "type_traits.hpp"
#include "type_name.hpp"
#include "overload_resolution.hpp"
#include "to_json_key.hpp"
#include "value.hpp"
#include "error.hpp"
#include <utility>
#include <sstream>

namespace json {
namespace detail {
struct to_json_algo;

struct to_json_impl {
private:
    template<typename T, typename U = Unqualified<T>, EnableIf<is_regular_serialisable<U>> = 0>
    value impl(choice<0>, T&& value) const {
        return ::json::value(std::forward<T>(value));
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_array_like<U>> = 0>
    value impl(choice<1>, T&& value) const {
        using value_type = typename array_value_type<U>::type;
        static_assert(is_serialisable<value_type>::value, "The array value type must be JSON serialisable.");
        array ret;
        for(auto&& x : std::forward<T>(value)) {
            ret.push_back(impl(select_overload{}, x));
        }
        return ret;
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_object_like<U>> = 0>
    value impl(choice<2>, T&& value) const {
        using value_type = typename U::mapped_type;
        static_assert(is_serialisable<value_type>::value, "The map's mapped type must be JSON serialisable.");
        object obj;
        for(auto&& p : std::forward<T>(value)) {
            obj.emplace(to_json_key(p.first), impl(select_overload{}, p.second));
        }
        return obj;
    }

    template<typename T, typename U = Unqualified<T>>
    value impl(otherwise, T&& value) const {
        DependOn<to_json_algo, U> algo {};
        json_schema<U> {}(algo, std::forward<T>(value));
        return std::move(algo).result;
    }
public:
    template<typename T>
    auto operator()(T&& value) const -> decltype(impl(select_overload{}, std::forward<T>(value))) {
        return impl(select_overload{}, std::forward<T>(value));
    }
};

struct from_json_algo;

struct from_json_impl {
private:
    template<typename T, EnableIf<is_json<T>> = 0>
    void impl(choice<0>, const value& v, T& dest) const {
        if(!v.is<T>()) {
            std::ostringstream fmt;
            fmt << "expected " << type_name<T>::value << ", received " << v.type_name() << " instead";
            throw from_json_error(std::move(fmt).str());
        }

        dest = v.as<T>();
    }

    void impl(choice<1>, const value& v, value& dest) const {
        dest = v;
    }

    template<typename T, EnableIf<is_array_like<T>, Not<std::is_array<T>>> = 0>
    void impl(choice<2>, const value& v, T& dest) const {
        using value_type = typename T::value_type;
        static_assert(is_deserialisable<value_type>::value, "The array's value type must be JSON deserialisable.");
        auto&& arr = v.as<array>();
        dest.clear();
        for(array::size_type i = 0; i < arr.size(); ++i) {
            try {
                dest.emplace_back(call<value_type>(arr[i]));
            }
            catch(from_json_error& e) {
                std::ostringstream fmt;
                fmt << "at array element " << i << ": " << std::move(e).message;
                e.message = std::move(fmt).str();
                throw;
            }
        }
    }

    template<typename T, EnableIf<is_object_like<T>> = 0>
    void impl(choice<3>, const value& v, T& dest) const {
        using mapped_type = typename T::mapped_type;
        using key_type = typename T::key_type;
        static_assert(is_deserialisable<mapped_type>::value, "The map's mapped type must be JSON deserialisable.");
        static_assert(is_string<key_type>::value, "The map's key must be a string.");
        dest.clear();
        auto&& obj = v.as<object>();
        for(auto&& p : obj) {
            try {
                dest.emplace(p.first, call<mapped_type>(p.second));
            }
            catch(from_json_error& e) {
                std::ostringstream fmt;
                fmt << "at object key \"" << p.first << "\": " << std::move(e).message;
                e.message = std::move(fmt).str();
                throw;
            }
        }
    }

    template<typename T>
    void impl(otherwise, const value& v, T& dest) const {
        if(!v.is<object>()) {
            throw from_json_error("expected object, received " + v.type_name() + " instead");
        }

        DependOn<from_json_algo, T> algo { v.as<object>() };
        json_schema<T>{}(algo, dest);
    }
public:
    template<typename T>
    T call(const value& v) const {
        T result;
        impl(select_overload{}, v, result);
        return result;
    }

    template<typename T>
    void call(const value& v, T& result) const {
        impl(select_overload{}, v, result);
    }
};
} // detail

constexpr detail::to_json_impl to_json{};
constexpr detail::from_json_impl from_json_caller{};

template<typename T>
inline void from_json(const value& v, T& result) {
    from_json_caller.call(v, result);
}

template<typename T>
inline auto from_json(const value& v) -> decltype(from_json_caller.call<T>(v)) {
    return from_json_caller.call<T>(v);
}

namespace detail {
// the actual algorithms
struct to_json_algo {
    object result;

    template<typename Source>
    void member(const char* name, const Source& source) {
        result.emplace(name, to_json(source));
    }
};

struct from_json_algo {
    object obj;

    template<typename Value>
    void member(const char* name, Value& value) const {
        auto&& js = value_at(name);
        try {
            from_json(js, value);
        }
        catch(from_json_error& e) {
            std::ostringstream fmt;
            fmt << "bad member '" << name << "': " << std::move(e).message;
            e.message = std::move(fmt).str();
            throw;
        }
    }

    const value& value_at(const char* name) const {
        auto it = obj.find(name);
        if(it == obj.end()) {
            std::ostringstream fmt;
            fmt << "missing member '" << name << '\'';
            throw from_json_error{ std::move(fmt).str() };
        }
        return it->second;
    }

    bool has_key(const char* name) const {
        return obj.count(name);
    }
};
} // detail
} // json

#endif // JSONPP_CONVERT_HPP
