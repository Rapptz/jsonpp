#include <catch.hpp>
#include <jsonpp/dump.hpp>
#include <jsonpp/value.hpp>
#include <cstddef>

struct jsonpp_breaker {
    jsonpp_breaker(std::nullptr_t) {

    }
};

json::value to_json(jsonpp_breaker) {
    return json::value(1);
}

TEST_CASE("Regressions", "[regressions]") {
    SECTION("issue-15") {
        REQUIRE(json::dump_string(nullptr) == "null");
    }
}
