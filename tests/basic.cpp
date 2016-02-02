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
    json::format_options minify;
    minify.indent = 0;
    minify.flags = minify.minify;

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
        REQUIRE(json::dump_string(v, minify) == "10");

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
        REQUIRE(json::dump_string(v, minify) == "2.14567e+101");

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
        REQUIRE(json::dump_string(v, minify) == "-10");
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
        REQUIRE(json::dump_string(v, minify) == "10");

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
        REQUIRE(json::dump_string(v, minify) == "1.23456");
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
    json::format_options minify;
    minify.indent = 0;
    minify.flags = minify.minify;

    SECTION("empty string") {
        REQUIRE_NOTHROW(json::parse("\t\n\"\"\n\n", v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<bool>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(json::dump_string(v, minify) == "\"\"");

        auto&& str = v.as<std::string>();
        REQUIRE(str.empty());
    }

    SECTION("escaped strings") {
        static const auto& basic_escaped_test = R"("\"")";
        REQUIRE_NOTHROW(json::parse(basic_escaped_test, v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<bool>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(json::dump_string(v, minify) == basic_escaped_test);

        auto&& str1 = v.as<std::string>();
        REQUIRE(str1.size() == 1);
        REQUIRE(str1.back() == '"');

        static const auto& hex_escapes = R"("\t\n\b\"\u2000\u1234")";
        REQUIRE_NOTHROW(json::parse(hex_escapes, v));
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(v.is<std::string>());
        REQUIRE(v.is<const char*>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(json::dump_string(v, minify) == "\"\\t\\n\\b\\\"\xe2\x80\x80\xe1\x88\xb4\"");

        auto&& str2 = v.as<std::string>();
        REQUIRE(str2.size() == 10);
        REQUIRE(str2 == u8"\t\n\b\"\u2000\u1234");
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
        REQUIRE(json::dump_string(v, minify) == "\"hello world\"");

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
        REQUIRE(json::dump_string(v, minify) == "\"hello\"");

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
    json::format_options minify;
    minify.indent = 0;
    minify.flags = minify.minify;

    SECTION("empty array") {
        REQUIRE_NOTHROW(json::parse("\t\t\n\t\n[]\n\t\t\n\n", v));
        REQUIRE(v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<bool>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(json::dump_string(v, minify) == "[]");
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
        REQUIRE(json::dump_string(v, minify) == "[10]");
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
        REQUIRE(json::dump_string(v, minify) == "[null,\"hello\",10,\"wow\"]");
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
        REQUIRE(json::dump_string(v, minify) == "[10,null,\"hello\",1.23456]");

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
    json::format_options minify;
    minify.indent = 0;
    minify.flags = minify.minify;

    SECTION("parsing") {
        REQUIRE_NOTHROW(json::parse("\n\n\tnull\n\n\t", v));
        REQUIRE(v.is<json::null>());
        REQUIRE(v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(json::dump_string(v, minify) == "null");

        auto&& x = v.as<json::null>();
        REQUIRE((x == nullptr));

        REQUIRE_NOTHROW(json::parse("\t\n\t\n\ttrue\n\t\n\t", v));
        REQUIRE(v.is<bool>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.as<bool>());
        REQUIRE(json::dump_string(v, minify) == "true");

        REQUIRE_NOTHROW(json::parse("\n\n\t\nfalse\n\t\n", v));
        REQUIRE(v.is<bool>());
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(!v.as<bool>());
        REQUIRE(json::dump_string(v, minify) == "false");
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
        REQUIRE(json::dump_string(v, minify) == "null");

        v = true;
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.is<bool>());
        REQUIRE(json::dump_string(v, minify) == "true");
        REQUIRE(v.as<bool>());

        v = false;
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(!v.is<json::object>());
        REQUIRE(v.is<bool>());
        REQUIRE(json::dump_string(v, minify) == "false");
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

TEST_CASE("comments", "[basic-comments]") {
    json::value v;
    json::format_options minify;
    minify.indent = 0;
    minify.flags = minify.minify;

    SECTION("empty objects") {
        REQUIRE_NOTHROW(json::parse<json::extensions::comments>("\t// A comment does not affect anything\n\t\n{     \t\n }/* Truly it does not */\n\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(json::dump_string(v, minify) == "{}");

        auto&& obj = v.as<json::object>();
        REQUIRE(obj.empty());
    }

    SECTION("one element object") {
        REQUIRE_NOTHROW(json::parse<json::extensions::comments>("\n\t\n{ \"hello\"/*the start */: 10 }\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(json::dump_string(v, minify) == "{\"hello\":10}");

        auto&& obj = v.as<json::object>();
        REQUIRE(!obj.empty());
        REQUIRE(obj.size() == 1);
        REQUIRE(obj.count("hello"));
        REQUIRE(obj.at("hello").is<int>());
        REQUIRE(obj.at("hello").as<int>() == 10);
    }

    SECTION("regular objects") {
        REQUIRE_NOTHROW(json::parse<json::extensions::comments>("// Some regular objects\n\t/* really just some regular objects! */\n\t{\"hello\" // They can go anywhere?\n:10, \"world\": null, \"test\":/*\"dahdjwakd\": invalid invalid*/ \"work\"}//Seems like it's alright!\n\t\n", v));
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
        REQUIRE(obj.at("hello").is<int>());
        REQUIRE(obj.at("hello").as<int>() == 10);
        REQUIRE(obj.count("world"));
        REQUIRE(obj.at("world").is<json::null>());
        REQUIRE((obj.at("world").as<json::null>() == nullptr));
        REQUIRE(obj.count("test"));
        REQUIRE(obj.at("test").is<std::string>());
        REQUIRE(obj.at("test").as<std::string>() == "work");
    }

    SECTION("invalid") {
        REQUIRE_THROWS(json::parse<json::extensions::comments>("// coments don't affect anything\n{ \"hello\" null }", v));
        REQUIRE_THROWS(json::parse<json::extensions::comments>("{ \"hello\": null/*,\n*/ goodbye: true }", v));
        REQUIRE_THROWS(json::parse<json::extensions::comments>("{ }/* an unfinished comment block is an error", v));
        REQUIRE_THROWS(json::parse<json::extensions::comments>("/* an unfinished comment at the start also fails{ }", v));
    }
}

TEST_CASE("objects", "[basic-objects]") {
    json::value v;
    json::format_options minify;
    minify.indent = 0;
    minify.flags = minify.minify;

    SECTION("empty objects") {
        REQUIRE_NOTHROW(json::parse("\t\n\t\n{     \t\n }\n\n\t\n", v));
        REQUIRE(!v.is<json::null>());
        REQUIRE(!v.is<std::nullptr_t>());
        REQUIRE(!v.is<json::array>());
        REQUIRE(!v.is<double>());
        REQUIRE(!v.is<std::string>());
        REQUIRE(v.is<json::object>());
        REQUIRE(!v.is<bool>());
        REQUIRE(json::dump_string(v, minify) == "{}");

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
        REQUIRE(json::dump_string(v, minify) == "{\"hello\":10}");

        auto&& obj = v.as<json::object>();
        REQUIRE(!obj.empty());
        REQUIRE(obj.size() == 1);
        REQUIRE(obj.count("hello"));
        REQUIRE(obj.at("hello").is<int>());
        REQUIRE(obj.at("hello").as<int>() == 10);
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
        REQUIRE(obj.at("hello").is<int>());
        REQUIRE(obj.at("hello").as<int>() == 10);
        REQUIRE(obj.count("world"));
        REQUIRE(obj.at("world").is<json::null>());
        REQUIRE((obj.at("world").as<json::null>() == nullptr));
        REQUIRE(obj.count("test"));
        REQUIRE(obj.at("test").is<std::string>());
        REQUIRE(obj.at("test").as<std::string>() == "work");
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
        REQUIRE(obj.at("hello").is<int>());
        REQUIRE(obj.at("hello").as<int>() == 10);
        REQUIRE(obj.count("world"));
        REQUIRE(obj.at("world").is<json::null>());
        REQUIRE((obj.at("world").as<json::null>() == nullptr));
        REQUIRE(obj.count("test"));
        REQUIRE(obj.at("test").is<std::string>());
        REQUIRE(obj.at("test").as<std::string>() == "work");
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
