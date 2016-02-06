#include <catch.hpp>
#include <jsonpp/to_json_key.hpp>
#include <string>

namespace ns {
struct json_key_test {
    int member;
};

std::string to_json_key(const json_key_test& x) {
    return std::to_string(x.member);
}
} // ns


TEST_CASE("to_json_key", "[serialisation][key]") {
    SECTION("built-in cases") {
        REQUIRE(json::to_json_key(true) == "true");
        REQUIRE(json::to_json_key(false) == "false");
        REQUIRE(json::to_json_key(10) == "10");
        REQUIRE(json::to_json_key(0) == "0");
        REQUIRE(json::to_json_key(1.123456) == "1.123456");
        REQUIRE(json::to_json_key(2147483647) == "2147483647");
        REQUIRE(json::to_json_key('c') == "c");
        REQUIRE(json::to_json_key(std::string("hello world")) == "hello world");
    }

    SECTION("customisable") {
        REQUIRE(json::to_json_key(ns::json_key_test{10}) == "10");
        REQUIRE(json::to_json_key(ns::json_key_test{50}) == "50");
        ns::json_key_test first = { 40 };
        REQUIRE(json::to_json_key(first) == "40");
        using namespace ns;
        REQUIRE(json::to_json_key(json_key_test{20}) == "20");
        json_key_test second = { 50 };
        REQUIRE(json::to_json_key(second) == "50");
    }
}
