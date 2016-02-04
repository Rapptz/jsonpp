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

#ifndef JSONPP_VALUE_HPP
#define JSONPP_VALUE_HPP

#include "type_traits.hpp"
#include "dump.hpp"
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <iosfwd>

namespace json {
class value {
public:
    using object = std::map<std::string, value>;
    using array  = std::vector<value>;
private:
    union storage_t {
        double number;
        bool boolean;
        std::string* str;
        array* arr;
        object* obj;
    } storage;
    type storage_type;

    void copy(const value& other) {
        switch(other.storage_type) {
        case type::array:
            storage.arr = new array(*(other.storage.arr));
            break;
        case type::string:
            storage.str = new std::string(*(other.storage.str));
            break;
        case type::object:
            storage.obj = new object(*(other.storage.obj));
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        default:
            break;
        }
        storage_type = other.storage_type;
    }
public:
    value() JSONPP_NOEXCEPT: storage_type(type::null) {}
    value(null) JSONPP_NOEXCEPT: storage_type(type::null) {}

    ~value() {
        clear();
    }

    value(double v) JSONPP_NOEXCEPT: storage_type(type::number) {
        storage.number = v;
    }

    template<typename T, EnableIf<is_bool<T>, Not<is_string<T>>> = 0>
    value(const T& b) JSONPP_NOEXCEPT: storage_type(type::boolean) {
        storage.boolean = b;
    }

    template<typename T, EnableIf<is_string<T>, Not<is_bool<T>>> = 0>
    value(const T& str): storage_type(type::string) {
        storage.str = new std::string(str);
    }

    value(const char* str, std::size_t sz): storage_type(type::string) {
        storage.str = new std::string(str, sz);
    }

    value(const char* str, const char* strend): storage_type(type::string) {
        storage.str = new std::string(str, strend);
    }

    template<typename T, EnableIf<has_to_json<T>, Not<is_string<T>>, Not<is_bool<T>>> = 0>
    value(const T& t): value(to_json(t)) {}

    value(const array& arr): storage_type(type::array) {
        storage.arr = new array(arr);
    }

    value(const object& obj): storage_type(type::object) {
        storage.obj = new object(obj);
    }

    value(std::initializer_list<array::value_type> l): storage_type(type::array) {
        storage.arr = new array(l.begin(), l.end());
    }

    value(const value& other) {
        copy(other);
    }

    value(value&& other) JSONPP_NOEXCEPT {
        switch(other.storage_type) {
        case type::array:
            storage.arr = other.storage.arr;
            other.storage.arr = nullptr;
            break;
        case type::string:
            storage.str = other.storage.str;
            other.storage.str = nullptr;
            break;
        case type::object:
            storage.obj = other.storage.obj;
            other.storage.obj = nullptr;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        default:
            break;
        }

        storage_type = other.storage_type;
        other.storage_type = type::null;
    }

    template<typename T, EnableIf<has_to_json<T>, Not<is_string<T>>, Not<is_bool<T>>> = 0>
    value& operator=(const T& t) {
        *this = to_json(t);
        return *this;
    }

    value& operator=(const value& other) {
        clear();
        copy(other);
        return *this;
    }

    value& operator=(value&& other) JSONPP_NOEXCEPT {
        clear();
        switch(other.storage_type) {
        case type::array:
            storage.arr = other.storage.arr;
            other.storage.arr = nullptr;
            break;
        case type::string:
            storage.str = other.storage.str;
            other.storage.str = nullptr;
            break;
        case type::object:
            storage.obj = other.storage.obj;
            other.storage.obj = nullptr;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        default:
            break;
        }

        storage_type = other.storage_type;
        other.storage_type = type::null;
        return *this;
    }

    std::string type_name() const {
        switch(storage_type) {
        case type::array:
            return "array";
        case type::string:
            return "string";
        case type::object:
            return "object";
        case type::number:
            return "number";
        case type::boolean:
            return "boolean";
        case type::null:
            return "null";
        default:
            return "unknown";
        }
    }

    void clear() JSONPP_NOEXCEPT {
        switch(storage_type) {
        case type::array:
            delete storage.arr;
            break;
        case type::string:
            delete storage.str;
            break;
        case type::object:
            delete storage.obj;
            break;
        default:
            break;
        }
        storage_type = type::null;
    }

    template<typename T, EnableIf<is_string<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::string;
    }

    template<typename T, EnableIf<is_null<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::null;
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::number;
    }

    template<typename T, EnableIf<is_bool<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::boolean;
    }

    template<typename T, EnableIf<is_object<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::object;
    }

    template<typename T, EnableIf<is_array<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::array;
    }

    template<typename T, DisableIf<is_json<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return false;
    }

    template<typename T, EnableIf<std::is_same<T, const char*>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return storage.str->c_str();
    }

    template<typename T, EnableIf<std::is_same<T, std::string>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return *(storage.str);
    }

    template<typename T, EnableIf<is_null<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return {};
    }

    template<typename T, EnableIf<is_bool<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return storage.boolean;
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return storage.number;
    }

    template<typename T, EnableIf<is_object<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return *(storage.obj);
    }

    template<typename T, EnableIf<is_array<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return *(storage.arr);
    }

    template<typename T, DisableIf<is_json<T>> = 0>
    T as() const {
        static_assert(dependent_false<T>::value, "calling value::as<T>() on an invalid type (use a json type instead)");
        return {};
    }

    template<typename T>
    T as(Identity<T>&& def) const {
        return is<T>() ? as<T>() : std::forward<T>(def);
    }

    template<typename T, EnableIf<is_string<T>> = 0>
    value operator[](const T& str) const {
        if(!is<object>()) {
            return {};
        }

        auto it = storage.obj->find(str);
        if(it != storage.obj->end()) {
            return it->second;
        }
        return {};
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    value operator[](const T& index) const {
        if(!is<array>()) {
            return {};
        }

        auto&& arr = *storage.arr;

        if(static_cast<size_t>(index) < arr.size()) {
            return arr[index];
        }
        return {};
    }

    template<typename OStream>
    friend OStream& dump(OStream& out, const value& val, format_options opt = {}) {
        switch(val.storage_type) {
        case type::array:
            return dump(out, *val.storage.arr, opt);
        case type::string:
            return dump(out, *val.storage.str, opt);
        case type::object:
            return dump(out, *val.storage.obj, opt);
        case type::boolean:
            return dump(out, val.storage.boolean, opt);
        case type::number:
            return dump(out, val.storage.number, opt);
        case type::null:
            return dump(out, nullptr, opt);
        default:
            return out;
        }
    }
};

template<typename OStream>
inline OStream& dump(OStream& out, const value& val, format_options opt);

using array  = value::array;
using object = value::object;

template<typename T>
inline auto value_cast(const value& v) -> decltype(v.as<T>()) {
    return v.as<T>();
}

template<typename T>
inline auto value_cast(const value& v, T&& def) -> decltype(v.as<Unqualified<T>>(std::forward<T>(def))) {
    return v.as<Unqualified<T>>(std::forward<T>(def));
}
} // json

#endif // JSONPP_VALUE_HPP
