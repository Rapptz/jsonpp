// The MIT License (MIT)

// Copyright (c) 2014 Rapptz

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
#include <jsonpp/parser.hpp>

TEST_CASE("numbers", "[basic-numbers]") {
    json::parser p;
    json::value v;

    SECTION("parsing") {
        REQUIRE_NOTHROW(p.parse("\t\n\n10", v));
        REQUIRE(v.is<int>());
        REQUIRE(v.is<double>());
        REQUIRE(v.is<float>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<const char*>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.as<int>() == 10);
        REQUIRE(v.to_string() == "10");

        REQUIRE_NOTHROW(p.parse("\t\t\n2.14567e+101", v));
        REQUIRE(v.is<int>());
        REQUIRE(v.is<double>());
        REQUIRE(v.is<float>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<const char*>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "2.14567e+101");
    }

    SECTION("writing") {
        v = 10;
        REQUIRE(v.is<int>());
        REQUIRE(v.is<double>());
        REQUIRE(v.is<float>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<const char*>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.as<int>() == 10);
        REQUIRE(v.to_string() == "10");

        v = 1.23456;
        REQUIRE(v.is<int>());
        REQUIRE(v.is<double>());
        REQUIRE(v.is<float>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<const char*>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "1.23456");
    }
}

TEST_CASE("strings", "[basic-strings]") {
    json::parser p;
    json::value v;

    SECTION("empty string") {
        REQUIRE_NOTHROW(p.parse("\t\n\"\"\n\n", v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "\"\"");

        auto&& str = v.as<std::string>();
        REQUIRE(str.empty());
    }

    SECTION("regular string") {
        REQUIRE_NOTHROW(p.parse("\t\n\n\n\t\n\n   \"hello world\"\n\t\n\n", v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "\"hello world\"");

        auto&& str = v.as<std::string>();
        REQUIRE(!str.empty());
        REQUIRE(str.size() == 11);
        REQUIRE(str == "hello world");
    }

    SECTION("writing") {
        v = "hello";
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "\"hello\"");

        auto&& str = v.as<std::string>();
        REQUIRE(!str.empty());
        REQUIRE(str.size() == 5);
        REQUIRE(str == "hello");
    }
}

TEST_CASE("arrays", "[basic-arrays]") {
    json::parser p;
    json::value v;

    SECTION("empty array") {
        REQUIRE_NOTHROW(p.parse("\t\t\n\t\n[]\n\t\t\n\n", v));
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "[]");
        auto&& arr = v.as<json::array>();
        REQUIRE(arr.empty());
    }

    SECTION("single element array") {
        REQUIRE_NOTHROW(p.parse("[10]\n\t\n", v));
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "[10]");
        auto&& arr = v.as<json::array>();
        REQUIRE(!arr.empty());
        REQUIRE(arr.size() == 1);
        REQUIRE(arr.back().is<int>());
        REQUIRE(arr.back().as<int>() == 10);
    }

    SECTION("regular array") {
        REQUIRE_NOTHROW(p.parse("\t\n\n\t\n  [null, \"hello\", 10.3e-10, \"wow\"]\t\n\t", v));
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string(4) == "[null, \"hello\", 1.03e-009, \"wow\"]");
        auto&& arr = v.as<json::array>();
        REQUIRE(!arr.empty());
        REQUIRE(arr.size() == 4);
        REQUIRE(arr.back().is<std::string>());
        REQUIRE(arr.back().as<std::string>() == "wow");
        REQUIRE(arr.front().is<json::null>());
    }

    SECTION("writing") {
        v = { 10, nullptr, "hello", 1.23456 };
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "[10, null, \"hello\", 1.23456]");

        auto&& arr = v.as<json::array>();
        REQUIRE(!arr.empty());
        REQUIRE(arr.size() == 4);
        REQUIRE(arr.back().is<double>());
        REQUIRE(arr.front().is<int>());
        REQUIRE(arr.front().as<int>() == 10);
        REQUIRE(arr[1].is<json::null>());
        REQUIRE(arr[2].is<std::string>());
        REQUIRE(arr[2].as<std::string>() == "hello");
    }
}

TEST_CASE("null", "[basic-null]") {
    json::parser p;
    json::value v;

    SECTION("parsing") {
        REQUIRE_NOTHROW(p.parse("null", v));
        REQUIRE(v.is<json::null>());
        REQUIRE(v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "null");

        auto&& x = v.as<json::null>();
        REQUIRE(x == nullptr);

        REQUIRE_THROWS(p.parse("nulle", v));
        REQUIRE_THROWS(p.parse("enull", v));
    }

    SECTION("writing") {
        v = nullptr;
        REQUIRE(v.is<json::null>());
        REQUIRE(v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "null");
    }
}

TEST_CASE("objects", "[basic-objects]") {
    json::parser p;
    json::value v;

    SECTION("empty objects") {
        REQUIRE_NOTHROW(p.parse("\t\n\t\n{     \t\n }\n\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(v.to_string() == "{}");

        auto&& obj = v.as<json::object>();
        REQUIRE(obj.empty());
    }

    SECTION("one element object") {
        REQUIRE_NOTHROW(p.parse("\n\t\n{ \"hello\": 10 }\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(v.to_string() == "{\"hello\": 10}");

        auto&& obj = v.as<json::object>();
        REQUIRE(!obj.empty());
        REQUIRE(obj.size() == 1);
        REQUIRE(obj.count("hello"));
        REQUIRE(obj["hello"].is<int>());
        REQUIRE(obj["hello"].as<int>() == 10);
    }

    SECTION("regular objects") {
        REQUIRE_NOTHROW(p.parse("\n\t\n\t{\"hello\":10, \"world\": null, \"test\": \"work\"}\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());

        auto&& obj = v.as<json::object>();
        REQUIRE(!obj.empty());
        REQUIRE(obj.size() == 3);
        REQUIRE(obj.count("hello"));
        REQUIRE(obj["hello"].is<int>());
        REQUIRE(obj["hello"].as<int>() == 10);
        REQUIRE(obj.count("world"));
        REQUIRE(obj["world"].is<json::null>());
        REQUIRE(obj["world"].as<json::null>() == nullptr);
        REQUIRE(obj.count("test"));
        REQUIRE(obj["test"].is<std::string>());
        REQUIRE(obj["test"].as<std::string>() == "work");
    }

    SECTION("writing") {
        v = json::object { { "hello", 10 }, { "world", nullptr }, { "test", "work" }};
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());

        auto&& obj = v.as<json::object>();
        REQUIRE(!obj.empty());
        REQUIRE(obj.size() == 3);
        REQUIRE(obj.count("hello"));
        REQUIRE(obj["hello"].is<int>());
        REQUIRE(obj["hello"].as<int>() == 10);
        REQUIRE(obj.count("world"));
        REQUIRE(obj["world"].is<json::null>());
        REQUIRE(obj["world"].as<json::null>() == nullptr);
        REQUIRE(obj.count("test"));
        REQUIRE(obj["test"].is<std::string>());
        REQUIRE(obj["test"].as<std::string>() == "work");
    }
}
