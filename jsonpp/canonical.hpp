// The MIT License (MIT)

// Copyright (c) 2016 Rapptz

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

#ifndef JSON_CANONICAL_HPP
#define JSON_CANONICAL_HPP

#include "jsonpp/value.hpp"

namespace json {

template<typename X, typename... Dummies> struct depend_on { using type = X; };
template<typename X, typename... Dummies> using depend_on_t = typename depend_on<X, Dummies...>::type;

template<typename Type>
struct canonical_recipe {};

struct to_json_algo;

struct canonical_to_json_type {
private:
    template<typename Type>
    using is_json = Or<is_null<Type>, is_bool<Type>, is_number<Type>, is_string<Type>, is_array<Type>, is_object<Type>>;

    template<typename Value, EnableIf<is_json<Value>> = 0>
    static value impl(Value const& value, int)
    { return value; }

    template<typename Value, DisableIf<is_json<Value>> = 0>
    static value impl(Value const& value, long)
    {
        depend_on_t<to_json_algo, Value> algo {};
        canonical_recipe<Value> {}(algo, value);
        return std::move(algo).result;
    }

public:
    template<typename Value>
    value operator()(Value const& value) const
    { return impl(value, 0); }
};
constexpr canonical_to_json_type canonical_to_json {};

struct to_json_algo {
    object result;

    template<typename Value>
    void member(const char* name, Value const& value)
    {
        result.insert({ name, canonical_to_json(value) });
    }
};

struct exception {
    std::string message;
};

struct from_json_algo;

template<typename Value>
struct canonical_from_json_type {
private:
    template<typename D>
    using is_json = Or<
        is_null<depend_on_t<Value, D>>,
        is_bool<depend_on_t<Value, D>>,
        is_number<depend_on_t<Value, D>>,
        is_string<depend_on_t<Value, D>>,
        is_array<depend_on_t<Value, D>>,
        is_object<depend_on_t<Value, D>>
    >;

    template<typename D = void, EnableIf<is_json<D>> = 0>
    static Value impl(value const& v, int)
    {
        if(!v.is<Value>()) {
            throw exception { "expected a thing, received " + v.type_name() + " instead" };
        }
        return v.as<Value>();
    }

    template<typename D = void, DisableIf<is_json<D>> = 0>
    static Value impl(value const& v, long)
    {
        if(!v.is<object>()) {
            throw exception { "excepted object, received a(n) " + v.type_name() + " instead" };
        }

        auto&& obj = v.as<object>();
        from_json_algo algo { obj };
        Value result;
        canonical_recipe<Value> {}(algo, result);
        return result;
    }

public:
    Value operator()(value const& v) const
    { return impl(v, 0); }
};
template<typename Value>
constexpr canonical_from_json_type<Value> canonical_from_json {};

struct from_json_algo {
    object obj;

    template<typename Value>
    void member(const char* name, Value& value) const
    {
        using namespace std::literals::string_literals;

        auto it = obj.find(name);
        if(it == obj.end()) {
            throw exception { "missing member '"s + name + "'" };
        }

        try {
            value = canonical_from_json<Value>(it->second);
        } catch(exception& e) {
            e.message = "bad member '"s + name + "': " + std::move(e).message;
            throw;
        }
    }
};

} // json

#endif // JSON_CANONICAL_HPP
