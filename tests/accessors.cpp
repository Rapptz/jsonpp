// The MIT License (MIT)

// Copyright (c) 2014 Cocophotos

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

#include <catch.hpp>
#include <jsonpp/value.hpp>

TEST_CASE("accessors", "[subscripts-accessors]") {
    json::value v = { json::object{ {"test", 1} }, nullptr, "my_string", 1.0 };
    json::value o = json::object{ {"key", "value"}, {"int", 1} };

    SECTION("get from json::array") {
        REQUIRE(v[0].is<json::object>());
        REQUIRE(v[1].is<json::null>());
        REQUIRE(v[2].is<std::string>());
        REQUIRE(v[3].is<double>());
        REQUIRE(v[3].is<float>());
        REQUIRE(v[3].is<int>());
        REQUIRE(v[4].is<json::null>()); // never throw, return json::null
    };

    SECTION("get from json::object") {
        REQUIRE(o["key"].is<std::string>());
        REQUIRE(o["int"].is<int>());
        REQUIRE(o["unexist"].is<json::null>()); // never throw, return json::null
    };

    SECTION("get from complex structure") {
        REQUIRE(v[0]["test"].is<int>());
        REQUIRE(v[0]["test"].as<int>() == 1);
    };
}
