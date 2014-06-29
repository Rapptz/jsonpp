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
    json::value v;

    SECTION("parsing") {
        REQUIRE_NOTHROW(json::parse("\t\n\n10", v));
        REQUIRE(v.is<int>());
        REQUIRE(v.is<double>());
        REQUIRE(v.is<float>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<const char*>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.as<int>() == 10);
        REQUIRE(v.to_string() == "10");

        REQUIRE_NOTHROW(json::parse("\t\t\n2.14567e+101", v));
        REQUIRE(v.is<int>());
        REQUIRE(v.is<double>());
        REQUIRE(v.is<float>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<const char*>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.to_string() == "2.14567e+101");

        REQUIRE_NOTHROW(json::parse("\t\n\n-10", v));
        REQUIRE(v.is<int>());
        REQUIRE(v.is<double>());
        REQUIRE(v.is<float>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<const char*>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.as<int>() == -10);
        REQUIRE(v.to_string() == "-10");
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
        REQUIRE(!v.is<bool>());
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
        REQUIRE(!v.is<bool>());
        REQUIRE(v.to_string() == "1.23456");
    }

    SECTION("invalid") {
        REQUIRE_THROWS(json::parse("10x12", v));
        REQUIRE_THROWS(json::parse("1'0", v));
        REQUIRE_THROWS(json::parse("0xDEADBEEF", v));
        REQUIRE_THROWS(json::parse("0b10110101", v));
    }
}

TEST_CASE("strings", "[basic-strings]") {
    json::value v;

    SECTION("empty string") {
        REQUIRE_NOTHROW(json::parse("\t\n\"\"\n\n", v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<bool>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "\"\"");

        auto&& str = v.as<std::string>();
        REQUIRE(str.empty());
    }

    SECTION("escaped strings") {
        REQUIRE_NOTHROW(json::parse(R"("\"")", v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<bool>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == R"("\"")");

        auto&& str1 = v.as<std::string>();
        REQUIRE(str1.size() == 2);
        REQUIRE(str1.back() == '"');
        REQUIRE(str1.front() == '\\');

        REQUIRE_NOTHROW(json::parse(R"("\t\n\v\b\"\u2000\u1234")", v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == R"("\t\n\v\b\"\u2000\u1234")");

        auto&& str2 = v.as<std::string>();
        REQUIRE(str2.size() == 22);
        REQUIRE(str2 == R"(\t\n\v\b\"\u2000\u1234)");
    }

    SECTION("regular string") {
        REQUIRE_NOTHROW(json::parse("\t\n\n\n\t\n\n   \"hello world\"\n\t\n\n", v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<bool>());
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
        REQUIRE(!v.is<bool>());
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

    SECTION("invalid") {
        REQUIRE_THROWS(json::parse("'hello'", v));
        REQUIRE_THROWS(json::parse("\"", v));
        REQUIRE_THROWS(json::parse("'", v));
        REQUIRE_THROWS(json::parse("''", v));
    }
}

TEST_CASE("arrays", "[basic-arrays]") {
    json::value v;

    SECTION("empty array") {
        REQUIRE_NOTHROW(json::parse("\t\t\n\t\n[]\n\t\t\n\n", v));
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<bool>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "[]");
        auto&& arr = v.as<json::array>();
        REQUIRE(arr.empty());
    }

    SECTION("single element array") {
        REQUIRE_NOTHROW(json::parse("[10]\n\t\n", v));
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<bool>());
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
        REQUIRE_NOTHROW(json::parse("\t\n\n\t\n  [null, \"hello\", 10, \"wow\"]\t\n\t", v));
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<bool>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.to_string() == "[null,\"hello\",10,\"wow\"]");
        auto&& arr = v.as<json::array>();
        REQUIRE(!arr.empty());
        REQUIRE(arr.size() == 4);
        REQUIRE(arr.back().is<std::string>());
        REQUIRE(arr.back().as<std::string>() == "wow");
        REQUIRE(arr.front().is<json::null>());
    }

    SECTION("packed arrays") {
        REQUIRE_NOTHROW(json::parse("[0,[0,[0],0],0]", v));
        REQUIRE(v.is<json::array>());

        auto&& arr1 = v.as<json::array>();
        REQUIRE(arr1.size() == 3);
        REQUIRE(arr1.back().is<int>());
        REQUIRE(arr1.back().as<int>() == 0);
        REQUIRE(arr1.front().is<int>());
        REQUIRE(arr1.front().as<int>() == 0);
        REQUIRE(arr1[1].is<json::array>());

        auto&& arr2 = arr1[1].as<json::array>();
        REQUIRE(arr2.size() == 3);
        REQUIRE(arr2.back().is<int>());
        REQUIRE(arr2.back().as<int>() == 0);
        REQUIRE(arr2.front().is<int>());
        REQUIRE(arr2.front().as<int>() == 0);
        REQUIRE(arr2[1].is<json::array>());

        auto&& arr3 = arr2[1].as<json::array>();
        REQUIRE(arr3.size() == 1);
        REQUIRE(arr3.back().is<int>());
        REQUIRE(arr3.back().as<int>() == 0);
    }

    SECTION("deep nesting") {
        REQUIRE_NOTHROW(json::parse("[[[[]]]]", v));
        REQUIRE(v.is<json::array>());

        auto&& arr1 = v.as<json::array>();
        REQUIRE(arr1.size() == 1);
        REQUIRE(arr1.back().is<json::array>());

        auto&& arr2 = arr1.back().as<json::array>();
        REQUIRE(arr2.size() == 1);
        REQUIRE(arr2.back().is<json::array>());

        auto&& arr3 = arr2.back().as<json::array>();
        REQUIRE(arr3.size() == 1);
        REQUIRE(arr3.back().is<json::array>());

        auto&& arr4 = arr3.back().as<json::array>();
        REQUIRE(arr4.empty());
    }

    SECTION("writing") {
        v = { 10, nullptr, "hello", 1.23456 };
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.to_string() == "[10,null,\"hello\",1.23456]");

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

    SECTION("invalid") {
        REQUIRE_THROWS(json::parse("[", v));
        REQUIRE_THROWS(json::parse("]", v));
        REQUIRE_THROWS(json::parse("[[[[]]", v));
        REQUIRE_THROWS(json::parse("[1, 2,]", v));
        REQUIRE_THROWS(json::parse("[1 2]", v));
        REQUIRE_THROWS(json::parse("[]]", v));
    }
}

TEST_CASE("null and bool", "[basic-null-bool]") {
    json::value v;

    SECTION("parsing") {
        REQUIRE_NOTHROW(json::parse("\n\n\tnull\n\n\t", v));
        REQUIRE(v.is<json::null>());
        REQUIRE(v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.to_string() == "null");

        auto&& x = v.as<json::null>();
        REQUIRE(x == nullptr);

        REQUIRE_NOTHROW(json::parse("\t\n\t\n\ttrue\n\t\n\t", v));
        REQUIRE(v.is<bool>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.as<bool>());
        REQUIRE(v.to_string() == "true");

        REQUIRE_NOTHROW(json::parse("\n\n\t\nfalse\n\t\n", v));
        REQUIRE(v.is<bool>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.as<bool>());
        REQUIRE(v.to_string() == "false");
    }

    SECTION("writing") {
        v = nullptr;
        REQUIRE(v.is<json::null>());
        REQUIRE(v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.to_string() == "null");

        v = true;
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.is<bool>());
        REQUIRE(v.to_string() == "true");
        REQUIRE(v.as<bool>());

        v = false;
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.is<bool>());
        REQUIRE(v.to_string() == "false");
        REQUIRE(!v.as<bool>());
    }

    SECTION("invalid") {
        REQUIRE_THROWS(json::parse("nulle", v));
        REQUIRE_THROWS(json::parse("enull", v));
        REQUIRE_THROWS(json::parse("null   null", v));
        REQUIRE_THROWS(json::parse("truee", v));
        REQUIRE_THROWS(json::parse("ffalse", v));
        REQUIRE_THROWS(json::parse("f'alse", v));
        REQUIRE_THROWS(json::parse("ttrue", v));
        REQUIRE_THROWS(json::parse("t        rue", v));
        REQUIRE_THROWS(json::parse("fa    l\nse", v));
        REQUIRE_THROWS(json::parse("nu\nll", v));
    }
}

TEST_CASE("objects", "[basic-objects]") {
    json::value v;

    SECTION("empty objects") {
        REQUIRE_NOTHROW(json::parse("\t\n\t\n{     \t\n }\n\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.to_string() == "{}");

        auto&& obj = v.as<json::object>();
        REQUIRE(obj.empty());
    }

    SECTION("one element object") {
        REQUIRE_NOTHROW(json::parse("\n\t\n{ \"hello\": 10 }\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(v.to_string() == "{\"hello\":10}");

        auto&& obj = v.as<json::object>();
        REQUIRE(!obj.empty());
        REQUIRE(obj.size() == 1);
        REQUIRE(obj.count("hello"));
        REQUIRE(obj["hello"].is<int>());
        REQUIRE(obj["hello"].as<int>() == 10);
    }

    SECTION("regular objects") {
        REQUIRE_NOTHROW(json::parse("\n\t\n\t{\"hello\":10, \"world\": null, \"test\": \"work\"}\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(!v.is<bool>());

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
        REQUIRE(!v.is<bool>());

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

    SECTION("invalid") {
        REQUIRE_THROWS(json::parse("{", v));
        REQUIRE_THROWS(json::parse("{ hello: null }", v));
        REQUIRE_THROWS(json::parse("{ \"hello: null }", v));
        REQUIRE_THROWS(json::parse("{ \"hello\" null }", v));
        REQUIRE_THROWS(json::parse("{ \"hello\": null goodbye: true }", v));
        REQUIRE_THROWS(json::parse("{}}", v));
        REQUIRE_THROWS(json::parse("{{ }", v));
    }
}
