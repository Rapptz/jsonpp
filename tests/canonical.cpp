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

TEST_CASE("canonical", "[types don't know#") {
    SECTION("canonical_to_json") {
        auto json = dump_string(json::canonical_to_json(thing { "barry", 42, {} }));

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

    SECTION("canonical_from_json") {
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
            auto result = json::canonical_from_json<thing>(val);
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
            REQUIRE_THROWS_AS( json::canonical_from_json<thing>(val), json::exception );

            std::string message;
            try {
                json::canonical_from_json<thing>(val);
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
            REQUIRE_THROWS_AS( json::canonical_from_json<thing>(val), json::exception );

            std::string message;
            try {
                json::canonical_from_json<thing>(val);
            } catch(json::exception& e) {
                message = std::move(e).message;
            }
            std::string const expected = "bad member 'dress': bad member 'is_it_blue': expected a thing, received number instead";
            REQUIRE( message == expected );
        }
    }
}
