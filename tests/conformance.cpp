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

#include <jsonpp/parser.hpp>
#include <catch.hpp>
#include <fstream>
#include <vector>
#include <string>

// conformance tests from the official json.org test suite
// the goal is to get all of these to pass parsing

TEST_CASE("Conformance", "[conformance]") {
    json::value v;

    // it sure would be nice if C++ had listdir or something
    std::vector<std::string> files = {
        "tests/valid/fail10.json",
        "tests/valid/fail11.json",
        "tests/valid/fail12.json",
        "tests/valid/fail13.json",
        "tests/valid/fail14.json",
        "tests/valid/fail15.json",
        "tests/valid/fail16.json",
        "tests/valid/fail17.json",
        "tests/valid/fail19.json",
        "tests/valid/fail2.json",
        "tests/valid/fail20.json",
        "tests/valid/fail21.json",
        "tests/valid/fail22.json",
        "tests/valid/fail23.json",
        "tests/valid/fail24.json",
        "tests/valid/fail25.json",
        "tests/valid/fail26.json",
        "tests/valid/fail27.json",
        "tests/valid/fail28.json",
        "tests/valid/fail29.json",
        "tests/valid/fail3.json",
        "tests/valid/fail30.json",
        "tests/valid/fail31.json",
        "tests/valid/fail32.json",
        "tests/valid/fail33.json",
        "tests/valid/fail4.json",
        "tests/valid/fail5.json",
        "tests/valid/fail6.json",
        "tests/valid/fail7.json",
        "tests/valid/fail8.json",
        "tests/valid/fail9.json",
        "tests/valid/pass1.json",
        "tests/valid/pass2.json",
        "tests/valid/pass3.json"
    };

    for(auto&& file : files) {
        std::ifstream in(file.c_str());
        INFO("Conformance file: " << file);
        REQUIRE(in.good());
        REQUIRE(in.is_open());
        if(file.find("fail") != file.npos) {
            REQUIRE_THROWS(json::parse(in, v));
        }
        else {
            REQUIRE_NOTHROW(json::parse(in, v));
        }
    }
}
