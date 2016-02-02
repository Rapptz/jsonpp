#include <catch.hpp>
#include <jsonpp/canonical.hpp>

#include <jsonpp/parser.hpp>

namespace {

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

} // namespace

namespace json {
template<> struct canonical_recipe<color>: color_recipe {};
template<> struct canonical_recipe<thing>: thing_recipe {};
} // json

TEST_CASE("canonical", "[canonical]") {
    SECTION("canonical_to_json") {
        {
            auto json = dump_string(json::canonical_to_json(thing { "barry", 42, {} }));

            constexpr auto& rawtext = R"end(
{
    "dress": {
        "is_it_blue": false
    },
    "name": "barry",
    "number_of_ants": 42
}
)end";
            std::string const expected { rawtext+1, std::end(rawtext)-2 };

            REQUIRE( json == expected );
        }
    };

    SECTION("canonical_from_json") {
        {
            constexpr auto& rawtext = R"end(
{
    "name": "barry",
    "dress": { "is_it_blue": true },
    "number_of_ants": -3,
    "leniency": null,
}
)end";
            std::string const payload { rawtext+1, std::end(rawtext)-2 };

            json::value val;
            json::parse(payload, val);
            auto result = json::canonical_from_json<thing>(val);
            REQUIRE( result == (thing { "barry", -3, { true } }) );
        }

        {
            constexpr auto& rawtext = R"end(
{
    "nome": "barry",
    "dress": { "is_it_blue": true },
    "number_of_ants": -3,
}
)end";
            std::string const payload { rawtext+1, std::end(rawtext)-2 };

            json::value val;
            json::parse(payload, val);
            REQUIRE_THROWS_AS( json::canonical_from_json<thing>(val), json::canonical_from_json_error );

            std::string message;
            try {
                json::canonical_from_json<thing>(val);
            } catch(json::canonical_from_json_error& e) {
                message = std::move(e).message;
            }
            REQUIRE( message == "missing member 'name'" );
        }

        {
            constexpr auto& rawtext = R"end(
{
    "name": "barry",
    "dress": { "is_it_blue": 4 },
    "number_of_ants": -3,
}
)end";
            std::string const payload { rawtext+1, std::end(rawtext)-2 };

            json::value val;
            json::parse(payload, val);
            REQUIRE_THROWS_AS( json::canonical_from_json<thing>(val), json::canonical_from_json_error );

            std::string message;
            try {
                json::canonical_from_json<thing>(val);
            } catch(json::canonical_from_json_error& e) {
                message = std::move(e).message;
            }
            std::string const expected = "bad member 'dress': bad member 'is_it_blue': expected a(n) boolean, received a(n) number instead";
            REQUIRE( message == expected );
        }
    }
}
