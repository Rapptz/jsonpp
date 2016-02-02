#include <catch.hpp>
#include <jsonpp.hpp>

#include <string>
#include <iostream>

namespace {

template<typename Type>
struct canonical_recipe {};

struct color {
    bool is_it_blue;
};
constexpr bool operator==(color const& lhs, color const& rhs)
{ return lhs.is_it_blue == rhs.is_it_blue; }

struct color_recipe {
    template<typename Algo>
    constexpr void operator()(Algo& algo, color& c) const
    {
        algo.member("is_it_blue", c.is_it_blue);
    }

    template<typename Algo>
    constexpr void operator()(Algo& algo, color const& c) const
    {
        algo.member("is_it_blue", c.is_it_blue);
    }
};
template<> struct canonical_recipe<color>: color_recipe {};

struct thing {
    std::string name;
    int number_of_ants;
    color dress;
};
bool operator==(thing const& lhs, thing const& rhs)
{ return std::tie(lhs.name, lhs.number_of_ants, lhs.dress) == std::tie(rhs.name, rhs.number_of_ants, rhs.dress); }

struct thing_recipe {
    template<typename Algo>
    constexpr void operator()(Algo& algo, thing& t) const
    {
        algo.member("name", t.name);
        algo.member("number_of_ants", t.number_of_ants);
        algo.member("dress", t.dress);
    }

    template<typename Algo>
    constexpr void operator()(Algo& algo, thing const& t) const
    {
        algo.member("name", t.name);
        algo.member("number_of_ants", t.number_of_ants);
        algo.member("dress", t.dress);
    }
};
template<> struct canonical_recipe<thing>: thing_recipe {};

} // namespace

/***** Implementation here *****/

namespace json {
namespace {

template<typename X, typename... Dummies> struct depend_on { using type = X; };
template<typename X, typename... Dummies> using depend_on_t = typename depend_on<X, Dummies...>::type;

struct to_json_algo;

struct extensible_to_json_type {
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
constexpr extensible_to_json_type extensible_to_json {};

struct to_json_algo {
    object result;

    template<typename Value>
    void member(const char* name, Value const& value)
    {
        result.insert({ name, extensible_to_json(value) });
    }
};

struct exception {
    std::string message;
};

struct from_json_algo;

template<typename Value>
struct extensible_from_json_type {
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
constexpr extensible_from_json_type<Value> extensible_from_json {};

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
            value = extensible_from_json<Value>(it->second);
        } catch(exception& e) {
            e.message = "bad member '"s + name + "': " + std::move(e).message;
            throw;
        }
    }
};

} // namespace
} // json

TEST_CASE("tdkh", "[types don't know#") {
    SECTION("extensible_to_json") {
        auto json = dump_string(json::extensible_to_json(thing { "barry", 42, {} }));

        constexpr auto& expected = R"end(
{
    "dress": {
        "is_it_blue": false
    },
    "name": "barry",
    "number_of_ants": 42
}
)end";

        REQUIRE( json == std::string(expected+1, std::end(expected)-2) );
    };

    SECTION("extensible_from_json") {
        {
            constexpr auto& payload = R"end(
{
    "name": "rebort",
    "dress": { "is_it_blue": true },
    "number_of_ants": -3,
}
)end";

            json::value val;
            json::parse(std::string { payload+1, std::end(payload)-2 }, val);
            auto result = json::extensible_from_json<thing>(val);
            REQUIRE( result == (thing { "rebort", -3, true }) );
        }

        {
            constexpr auto& payload = R"end(
{
    "nome": "robert",
    "dress": { "is_it_blue": true },
    "number_of_ants": -3,
}
)end";

            json::value val;
            json::parse(std::string { payload+1, std::end(payload)-2 }, val);
            REQUIRE_THROWS_AS( json::extensible_from_json<thing>(val), json::exception );

            std::string message;
            try {
                json::extensible_from_json<thing>(val);
            } catch(json::exception& e) {
                message = std::move(e).message;
            }
            REQUIRE( message == "missing member 'name'" );
        }

        {
            constexpr auto& payload = R"end(
{
    "name": "robert",
    "dress": { "is_it_blue": 4 },
    "number_of_ants": -3,
}
)end";

            json::value val;
            json::parse(std::string { payload+1, std::end(payload)-2 }, val);
            REQUIRE_THROWS_AS( json::extensible_from_json<thing>(val), json::exception );

            std::string message;
            try {
                json::extensible_from_json<thing>(val);
            } catch(json::exception& e) {
                message = std::move(e).message;
            }
            std::string const expected = "bad member 'dress': bad member 'is_it_blue': expected a thing, received number instead";
            REQUIRE( message == expected );
        }
    }
}
