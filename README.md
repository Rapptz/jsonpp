## jsonpp

jsonpp is a header-only JSON parser and writer that is currently in development. It is a semi-strict
parser that throws exceptions for errors.

jsonpp is licensed with the MIT license.

## Features

- Easy to use with a simple API.
- No special null, array, or object types.
    - `json::null` is a type alias to `std::nullptr_t`.
    - `json::array` is `std::vector<json::value>`.
    - `json::object` is `std::map<std::string, json::value>`.
- Decently fast.
- No dependencies, only the standard library and a C++11 compiler.

## Example usage

#### Parsing

```cpp
#include <jsonpp/parser.hpp>
#include <iostream>

int main() {
    json::parser p;
    json::value v;
    try {
        p.parse("[null, \"hello\", 10.0]", v);
        if(v.is<json::array>()) {
            for(auto&& val : v.as<json::array>()) {
                std::cout << val.get<std::string>("stuff");
            }
        }
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
```

Output:
<pre>
stuff hello stuff
</pre>

#### Writing

```cpp
#include <jsonpp/value.hpp>
#include <iostream>

int main() {
    json::value v = { nullptr, "hello", 10 };
    json::value o = json::object{ {"key", "value"}, {"key2", 2}  };
    std::cout << v.to_string();

    //You can accessed the JSON object like this
    std::cout << v[0].to_string(); //Should return null
    std::cout << o["key"].to_string(); //Should return "value"
    std::cout << o["key2"].to_string(); //Should return "2"
}
```

Output:
<pre>
[null,"hello",10]
</pre>

## Quirks and Specification

- NaN and inf are currently allowed.
- Comments, e.g. `// stuff` is planned to be supported in the future.
- The string is only passed once.
- The parser is not destructive.
- The parser is recursive descent.
- `int` and `double` types are aliased to be the same.
- Forward slashes are not escaped.
- String is expected to be in UTF-8.
- Currently, no input validation is done on strings.
- Some errors are not caught but effort has been made to catch a lot of errors.
