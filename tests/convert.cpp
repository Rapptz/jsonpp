#include <catch.hpp>
#include <jsonpp/convert.hpp>
#include <jsonpp/dump.hpp>
#include <jsonpp/parser.hpp>
#include <tuple>

namespace ns {
struct user {
    std::string name;
    json::value avatar;
    std::vector<std::string> roles;
};

struct user_schema {
    template<typename Algo>
    void operator()(Algo& algo, const user& u) const {
        algo.member("name", u.name);
        algo.member("avatar", u.avatar);
        algo.member("roles", u.roles);
    }

    template<typename Algo>
    void operator()(Algo& algo, user& u) const {
        algo.member("name", u.name);
        algo.member("avatar", u.avatar);
        algo.member("roles", u.roles);
    }
};

inline bool operator==(const user& lhs, const user& rhs) JSONPP_NOEXCEPT {
    return std::tie(lhs.name, lhs.avatar, lhs.roles) == std::tie(rhs.name, rhs.avatar, rhs.roles);
}
} // ns

namespace json {
template<> struct json_schema<ns::user> : ns::user_schema {};
} // json

TEST_CASE("conversion", "[conversion]") {
    constexpr auto& rawtext = R"js(
{
    "avatar": null,
    "name": "Danny",
    "roles": [
        "Cool",
        "Programmer",
        "Admin"
    ]
})js";

    const std::string payload = { rawtext + 1, std::end(rawtext) - 1 };

    ns::user danny = {
        "Danny",
        nullptr,
        { "Cool", "Programmer", "Admin" }
    };

    SECTION("serialisation") {
        auto js = json::dump_string(json::to_json(danny));
        REQUIRE(js == payload);
    }

    SECTION("deserialisation") {
        json::value v;
        REQUIRE_NOTHROW(json::parse(rawtext, v));
        REQUIRE(v.is<json::object>());
        ns::user otherway;
        REQUIRE_NOTHROW(json::from_json(v, otherway));
        REQUIRE(danny == otherway);
    }
}
