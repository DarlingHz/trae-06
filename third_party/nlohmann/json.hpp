// MIT License
// 
// Copyright (c) 2013-2022 Niels Lohmann <https://nlohmann.me>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef NLOHMANN_JSON_HPP
#define NLOHMANN_JSON_HPP

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <ratio>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace nlohmann
{

class json;

namespace detail
{

struct string_t { };
struct non_string_t { };

template<typename T>
struct is_string : std::integral_constant<bool, std::is_same<T, std::string>::value || std::is_same<T, const char*>::value || std::is_same<T, char*>::value> { };

template<typename T>
struct is_wstring : std::integral_constant<bool, std::is_same<T, std::wstring>::value || std::is_same<T, const wchar_t*>::value || std::is_same<T, wchar_t*>::value> { };

} // namespace detail

// Forward declarations
template<typename T>
T from_json(const json& j);

class json
{
public:
    enum class value_t
    {
        null,
        boolean,
        number_integer,
        number_float,
        string,
        array,
        object
    };

private:
    value_t type_;
    std::variant<std::monostate, bool, int64_t, double, std::string, std::vector<json>, std::map<std::string, json>> data_;

public:
    json() : type_(value_t::null), data_(std::monostate()) {}
    json(std::nullptr_t) : type_(value_t::null), data_(std::monostate()) {}
    json(bool value) : type_(value_t::boolean), data_(value) {}
    json(int value) : type_(value_t::number_integer), data_(static_cast<int64_t>(value)) {}
    json(long value) : type_(value_t::number_integer), data_(static_cast<int64_t>(value)) {}
    json(long long value) : type_(value_t::number_integer), data_(value) {}
    json(unsigned int value) : type_(value_t::number_integer), data_(static_cast<int64_t>(value)) {}
    json(unsigned long value) : type_(value_t::number_integer), data_(static_cast<int64_t>(value)) {}
    json(unsigned long long value) : type_(value_t::number_integer), data_(static_cast<int64_t>(value)) {}
    json(double value) : type_(value_t::number_float), data_(value) {}
    json(const char* value) : type_(value_t::string), data_(std::string(value)) {}
    json(const std::string& value) : type_(value_t::string), data_(value) {}
    json(std::string&& value) : type_(value_t::string), data_(std::move(value)) {}
    json(const std::vector<json>& value) : type_(value_t::array), data_(value) {}
    json(std::vector<json>&& value) : type_(value_t::array), data_(std::move(value)) {}
    json(const std::map<std::string, json>& value) : type_(value_t::object), data_(value) {}
    json(std::map<std::string, json>&& value) : type_(value_t::object), data_(std::move(value)) {}

    value_t type() const { return type_; }
    bool is_null() const { return type_ == value_t::null; }
    bool is_boolean() const { return type_ == value_t::boolean; }
    bool is_number() const { return type_ == value_t::number_integer || type_ == value_t::number_float; }
    bool is_string() const { return type_ == value_t::string; }
    bool is_array() const { return type_ == value_t::array; }
    bool is_object() const { return type_ == value_t::object; }

    bool as_bool() const
    {
        if (!is_boolean()) throw std::runtime_error("Type mismatch: expected boolean");
        return std::get<bool>(data_);
    }

    int64_t as_integer() const
    {
        if (!is_number()) throw std::runtime_error("Type mismatch: expected number");
        if (type_ == value_t::number_integer) return std::get<int64_t>(data_);
        return static_cast<int64_t>(std::get<double>(data_));
    }

    double as_float() const
    {
        if (!is_number()) throw std::runtime_error("Type mismatch: expected number");
        if (type_ == value_t::number_float) return std::get<double>(data_);
        return static_cast<double>(std::get<int64_t>(data_));
    }

    const std::string& as_string() const
    {
        if (!is_string()) throw std::runtime_error("Type mismatch: expected string");
        return std::get<std::string>(data_);
    }

    const std::vector<json>& as_array() const
    {
        if (!is_array()) throw std::runtime_error("Type mismatch: expected array");
        return std::get<std::vector<json>>(data_);
    }

    std::vector<json>& as_array()
    {
        if (!is_array()) throw std::runtime_error("Type mismatch: expected array");
        return std::get<std::vector<json>>(data_);
    }

    const std::map<std::string, json>& as_object() const
    {
        if (!is_object()) throw std::runtime_error("Type mismatch: expected object");
        return std::get<std::map<std::string, json>>(data_);
    }

    std::map<std::string, json>& as_object()
    {
        if (!is_object()) throw std::runtime_error("Type mismatch: expected object");
        return std::get<std::map<std::string, json>>(data_);
    }

    template<typename T>
    T get() const
    {
        return from_json<T>(*this);
    }

    std::size_t size() const
    {
        if (is_array()) return as_array().size();
        if (is_object()) return as_object().size();
        throw std::runtime_error("Type mismatch: expected array or object");
    }

    bool empty() const
    {
        return size() == 0;
    }

    const json& operator[](std::size_t index) const
    {
        if (!is_array()) throw std::runtime_error("Type mismatch: expected array");
        const auto& arr = as_array();
        if (index >= arr.size()) throw std::out_of_range("Array index out of range");
        return arr[index];
    }

    json& operator[](std::size_t index)
    {
        if (!is_array()) throw std::runtime_error("Type mismatch: expected array");
        auto& arr = as_array();
        if (index >= arr.size()) throw std::out_of_range("Array index out of range");
        return arr[index];
    }

    const json& operator[](const std::string& key) const
    {
        if (!is_object()) throw std::runtime_error("Type mismatch: expected object");
        const auto& obj = as_object();
        auto it = obj.find(key);
        if (it == obj.end()) throw std::out_of_range("Object key not found");
        return it->second;
    }

    json& operator[](const std::string& key)
    {
        if (!is_object()) throw std::runtime_error("Type mismatch: expected object");
        auto& obj = as_object();
        return obj[key];
    }

    bool contains(const std::string& key) const
    {
        if (!is_object()) throw std::runtime_error("Type mismatch: expected object");
        const auto& obj = as_object();
        return obj.find(key) != obj.end();
    }

    void push_back(const json& value)
    {
        if (!is_array()) throw std::runtime_error("Type mismatch: expected array");
        auto& arr = as_array();
        arr.push_back(value);
    }

    void push_back(json&& value)
    {
        if (!is_array()) throw std::runtime_error("Type mismatch: expected array");
        auto& arr = as_array();
        arr.push_back(std::move(value));
    }

    void erase(std::size_t index)
    {
        if (!is_array()) throw std::runtime_error("Type mismatch: expected array");
        auto& arr = as_array();
        if (index >= arr.size()) throw std::out_of_range("Array index out of range");
        arr.erase(arr.begin() + index);
    }

    void erase(const std::string& key)
    {
        if (!is_object()) throw std::runtime_error("Type mismatch: expected object");
        auto& obj = as_object();
        obj.erase(key);
    }

    std::string dump(int indent = -1) const
    {
        std::ostringstream oss;
        dump(oss, indent, 0);
        return oss.str();
    }

private:
    void dump(std::ostream& os, int indent, int current_indent) const
    {
        switch (type_)
        {
            case value_t::null:
                os << "null";
                break;
            case value_t::boolean:
                os << (std::get<bool>(data_) ? "true" : "false");
                break;
            case value_t::number_integer:
                os << std::get<int64_t>(data_);
                break;
            case value_t::number_float:
                os << std::get<double>(data_);
                break;
            case value_t::string:
            {
                const auto& s = std::get<std::string>(data_);
                os << '"';
                for (char c : s)
                {
                    switch (c)
                    {
                        case '"': os << '\\' << '"'; break;
                        case '\\': os << '\\' << '\\'; break;
                        case '\b': os << '\\' << 'b'; break;
                        case '\f': os << '\\' << 'f'; break;
                        case '\n': os << '\\' << 'n'; break;
                        case '\r': os << '\\' << 'r'; break;
                        case '\t': os << '\\' << 't'; break;
                        default: os << c; break;
                    }
                }
                os << '"';
                break;
            }
            case value_t::array:
            {
                const auto& arr = as_array();
                os << '[';
                if (!arr.empty())
                {
                    if (indent >= 0)
                    {
                        os << '\n';
                        for (size_t i = 0; i < arr.size(); ++i)
                        {
                            if (i > 0) os << ',' << '\n';
                            os << std::string(current_indent + indent, ' ');
                            arr[i].dump(os, indent, current_indent + indent);
                        }
                        os << '\n' << std::string(current_indent, ' ');
                    }
                    else
                    {
                        for (size_t i = 0; i < arr.size(); ++i)
                        {
                            if (i > 0) os << ',';
                            arr[i].dump(os, indent, current_indent);
                        }
                    }
                }
                os << ']';
                break;
            }
            case value_t::object:
            {
                const auto& obj = as_object();
                os << '{';
                if (!obj.empty())
                {
                    if (indent >= 0)
                    {
                        os << '\n';
                        size_t i = 0;
                        for (const auto& [key, value] : obj)
                        {
                            if (i > 0) os << ',' << '\n';
                            os << std::string(current_indent + indent, ' ');
                            os << '"' << key << '"' << ": ";
                            value.dump(os, indent, current_indent + indent);
                            ++i;
                        }
                        os << '\n' << std::string(current_indent, ' ');
                    }
                    else
                    {
                        size_t i = 0;
                        for (const auto& [key, value] : obj)
                        {
                            if (i > 0) os << ',';
                            os << '"' << key << '"' << ":";
                            value.dump(os, indent, current_indent);
                            ++i;
                        }
                    }
                }
                os << '}';
                break;
            }
        }
    }
};

template<typename T>
T from_json(const json& j)
{
    if constexpr (std::is_same_v<T, bool>) return j.as_bool();
    else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) return static_cast<T>(j.as_integer());
    else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) return static_cast<T>(j.as_integer());
    else if constexpr (std::is_floating_point_v<T>) return static_cast<T>(j.as_float());
    else if constexpr (std::is_same_v<T, std::string>) return j.as_string();
    else throw std::runtime_error("Unsupported type conversion from json");
}



inline std::ostream& operator<<(std::ostream& os, const json& j)
{
    os << j.dump();
    return os;
}

inline std::istream& operator>>(std::istream& is, json& j)
{
    std::string s;
    is >> s;
    j = json(s);
    return is;
}

} // namespace nlohmann

#endif // NLOHMANN_JSON_HPP
