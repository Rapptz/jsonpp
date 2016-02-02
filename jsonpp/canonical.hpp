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

template<typename Type>
struct canonical_recipe {};

namespace detail {

struct exception {
    std::string message;
};

struct to_json_algo;

struct canonical_to_json_type {
private:
    template<typename Type>
    using is_json = Or<is_null<Type>, is_bool<Type>, is_number<Type>, is_string<Type>, is_array<Type>, is_object<Type>>;

    template<typename Source, EnableIf<is_json<Source>> = 0>
    static value impl(Source const& source, int)
    { return source; }

    template<typename Source, DisableIf<is_json<Source>> = 0>
    static value impl(Source const& source, long)
    {
        DependOn<to_json_algo, Source> algo {};
        canonical_recipe<Source> {}(algo, source);
        return std::move(algo).result;
    }

public:
    template<typename Source>
    value operator()(Source const& source) const
    { return impl(source, 0); }
};

} // detail

constexpr detail::canonical_to_json_type canonical_to_json {};

namespace detail {

struct to_json_algo {
    object result;

    template<typename Source>
    void member(const char* name, Source const& source)
    {
        result.insert({ name, canonical_to_json(source) });
    }
};

struct from_json_algo;

template<typename Dest>
struct canonical_from_json_type {
private:
    template<typename Dep>
    using is_json = Or<
        is_null<DependOn<Dest, Dep>>,
        is_bool<DependOn<Dest, Dep>>,
        is_number<DependOn<Dest, Dep>>,
        is_string<DependOn<Dest, Dep>>,
        is_array<DependOn<Dest, Dep>>,
        is_object<DependOn<Dest, Dep>>
    >;

    template<typename Dep = void, EnableIf<is_json<Dep>> = 0>
    static Dest impl(value const& v, int)
    {
        if(!v.is<Dest>()) {
            throw detail::exception { "expected a thing, received " + v.type_name() + " instead" };
        }
        return v.as<Dest>();
    }

    template<typename Dep = void, DisableIf<is_json<Dep>> = 0>
    static Dest impl(value const& v, long)
    {
        if(!v.is<object>()) {
            throw detail::exception { "expected object, received a(n) " + v.type_name() + " instead" };
        }

        auto&& obj = v.as<object>();
        DependOn<from_json_algo, Dep> algo { obj };
        Dest result;
        canonical_recipe<Dest> {}(algo, result);
        return result;
    }

public:
    Dest operator()(value const& v) const
    { return impl(v, 0); }
};

} // detail

template<typename Dest>
Dest canonical_from_json(value const& v)
{
    static constexpr detail::canonical_from_json_type<Dest> call;
    return call(v);
}

namespace detail {

struct from_json_algo {
    object obj;

    template<typename Value>
    void member(const char* name, Value& value) const
    {
        using namespace std::literals::string_literals;

        auto it = obj.find(name);
        if(it == obj.end()) {
            throw detail::exception { "missing member '"s + name + "'" };
        }

        try {
            value = canonical_from_json<Value>(it->second);
        } catch(detail::exception& e) {
            e.message = "bad member '"s + name + "': " + std::move(e).message;
            throw;
        }
    }
};

} // detail

} // json

#endif // JSON_CANONICAL_HPP
