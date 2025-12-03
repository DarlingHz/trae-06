#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace json {

class JsonValue {
public:
    enum Type { NULL_TYPE, BOOL, INT, DOUBLE, STRING, OBJECT, ARRAY };

    JsonValue() : type(NULL_TYPE) {}
    JsonValue(bool b) : type(BOOL), bool_val(b) {}
    JsonValue(int i) : type(INT), int_val(i) {}
    JsonValue(double d) : type(DOUBLE), double_val(d) {}
    JsonValue(const std::string& s) : type(STRING), string_val(s) {}
    JsonValue(std::string&& s) : type(STRING), string_val(std::move(s)) {}
    JsonValue(const char* s) : type(STRING), string_val(s) {}
    JsonValue(const std::map<std::string, JsonValue>& obj) : type(OBJECT), object_val(obj) {}
    JsonValue(std::map<std::string, JsonValue>&& obj) : type(OBJECT), object_val(std::move(obj)) {}
    JsonValue(const std::vector<JsonValue>& arr) : type(ARRAY), array_val(arr) {}
    JsonValue(std::vector<JsonValue>&& arr) : type(ARRAY), array_val(std::move(arr)) {}

    Type get_type() const { return type; }

    bool is_null() const { return type == NULL_TYPE; }
    bool is_bool() const { return type == BOOL; }
    bool is_int() const { return type == INT; }
    bool is_double() const { return type == DOUBLE; }
    bool is_string() const { return type == STRING; }
    bool is_object() const { return type == OBJECT; }
    bool is_array() const { return type == ARRAY; }

    bool as_bool() const { if (is_bool()) return bool_val; throw std::runtime_error("Not a bool"); }
    int as_int() const { if (is_int()) return int_val; throw std::runtime_error("Not an int"); }
    double as_double() const { if (is_double()) return double_val; throw std::runtime_error("Not a double"); }
    const std::string& as_string() const { if (is_string()) return string_val; throw std::runtime_error("Not a string"); }
    const std::map<std::string, JsonValue>& as_object() const { if (is_object()) return object_val; throw std::runtime_error("Not an object"); }
    const std::vector<JsonValue>& as_array() const { if (is_array()) return array_val; throw std::runtime_error("Not an array"); }

    const JsonValue& operator[](const std::string& key) const {
        if (is_object()) {
            auto it = object_val.find(key);
            if (it != object_val.end()) return it->second;
            throw std::runtime_error("Key not found: " + key);
        }
        throw std::runtime_error("Not an object");
    }

    JsonValue& operator[](const std::string& key) {
        if (is_object()) {
            return object_val[key];
        }
        throw std::runtime_error("Not an object");
    }

    const JsonValue& operator[](size_t index) const {
        if (is_array()) {
            if (index < array_val.size()) return array_val[index];
            throw std::runtime_error("Index out of bounds");
        }
        throw std::runtime_error("Not an array");
    }

    JsonValue& operator[](size_t index) {
        if (is_array()) {
            if (index < array_val.size()) return array_val[index];
            throw std::runtime_error("Index out of bounds");
        }
        throw std::runtime_error("Not an array");
    }

    bool has(const std::string& key) const {
        if (is_object()) {
            return object_val.find(key) != object_val.end();
        }
        throw std::runtime_error("Not an object");
    }

    const JsonValue& get(const std::string& key) const {
        if (is_object()) {
            auto it = object_val.find(key);
            if (it != object_val.end()) return it->second;
            throw std::runtime_error("Key not found: " + key);
        }
        throw std::runtime_error("Not an object");
    }

private:
    Type type;
    bool bool_val;
    int int_val;
    double double_val;
    std::string string_val;
    std::map<std::string, JsonValue> object_val;
    std::vector<JsonValue> array_val;
};

class Parser {
public:
    static JsonValue parse(const std::string& json_str) {
        pos = 0;
        s = json_str;
        skip_whitespace();
        return parse_value();
    }

private:
    static size_t pos;
    static std::string s;

    static void skip_whitespace() {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r')) {
            pos++;
        }
    }

    static JsonValue parse_value() {
        skip_whitespace();
        if (pos >= s.size()) throw std::runtime_error("Unexpected end of input");

        char c = s[pos];
        if (c == '{') return parse_object();
        if (c == '[') return parse_array();
        if (c == '"') return parse_string();
        if (c == 't' || c == 'f') return parse_bool();
        if (c == 'n') return parse_null();
        if (c == '-' || (c >= '0' && c <= '9')) return parse_number();

        throw std::runtime_error("Unexpected character: " + std::string(1, c));
    }

    static JsonValue parse_object() {
        pos++;
        std::map<std::string, JsonValue> obj;
        skip_whitespace();

        if (pos < s.size() && s[pos] == '}') {
            pos++;
            return JsonValue(obj);
        }

        while (pos < s.size()) {
            std::string key = parse_string().as_string();
            skip_whitespace();
            if (pos >= s.size() || s[pos] != ':') throw std::runtime_error("Expected ':'");
            pos++;
            JsonValue value = parse_value();
            obj[key] = value;
            skip_whitespace();
            if (pos < s.size() && s[pos] == '}') {
                pos++;
                return JsonValue(obj);
            }
            if (pos >= s.size() || s[pos] != ',') throw std::runtime_error("Expected ',' or '}'");
            pos++;
            skip_whitespace();
        }

        throw std::runtime_error("Unexpected end of input in object");
    }

    static JsonValue parse_array() {
        pos++;
        std::vector<JsonValue> arr;
        skip_whitespace();

        if (pos < s.size() && s[pos] == ']') {
            pos++;
            return JsonValue(arr);
        }

        while (pos < s.size()) {
            JsonValue value = parse_value();
            arr.push_back(value);
            skip_whitespace();
            if (pos < s.size() && s[pos] == ']') {
                pos++;
                return JsonValue(arr);
            }
            if (pos >= s.size() || s[pos] != ',') throw std::runtime_error("Expected ',' or ']'");
            pos++;
            skip_whitespace();
        }

        throw std::runtime_error("Unexpected end of input in array");
    }

    static JsonValue parse_string() {
        pos++;
        std::string str;
        while (pos < s.size() && s[pos] != '"') {
            if (s[pos] == '\\') {
                pos++;
                if (pos >= s.size()) throw std::runtime_error("Unexpected end of input in string");
                char escape = s[pos];
                switch (escape) {
                    case '"': str += '"'; break;
                    case '\\': str += '\\'; break;
                    case '/': str += '/'; break;
                    case 'b': str += '\b'; break;
                    case 'f': str += '\f'; break;
                    case 'n': str += '\n'; break;
                    case 'r': str += '\r'; break;
                    case 't': str += '\t'; break;
                    default: str += escape; break;
                }
            } else {
                str += s[pos];
            }
            pos++;
        }
        if (pos >= s.size() || s[pos] != '"') throw std::runtime_error("Unterminated string");
        pos++;
        return JsonValue(str);
    }

    static JsonValue parse_bool() {
        if (s.substr(pos, 4) == "true") {
            pos += 4;
            return JsonValue(true);
        } else if (s.substr(pos, 5) == "false") {
            pos += 5;
            return JsonValue(false);
        }
        throw std::runtime_error("Invalid boolean value");
    }

    static JsonValue parse_null() {
        if (s.substr(pos, 4) == "null") {
            pos += 4;
            return JsonValue();
        }
        throw std::runtime_error("Invalid null value");
    }

    static JsonValue parse_number() {
        size_t start = pos;
        if (s[pos] == '-') pos++;
        if (pos < s.size() && s[pos] == '0') {
            pos++;
            if (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) throw std::runtime_error("Invalid number format");
        } else if (pos < s.size() && (s[pos] >= '1' && s[pos] <= '9')) {
            pos++;
            while (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) pos++;
        }
        if (pos < s.size() && s[pos] == '.') {
            pos++;
            if (pos >= s.size() || !(s[pos] >= '0' && s[pos] <= '9')) throw std::runtime_error("Invalid number format");
            while (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) pos++;
        }
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
            pos++;
            if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) pos++;
            if (pos >= s.size() || !(s[pos] >= '0' && s[pos] <= '9')) throw std::runtime_error("Invalid number format");
            while (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) pos++;
        }
        std::string num_str = s.substr(start, pos - start);
        if (num_str.find('.') != std::string::npos || num_str.find('e') != std::string::npos || num_str.find('E') != std::string::npos) {
            return JsonValue(std::stod(num_str));
        } else {
            return JsonValue(std::stoi(num_str));
        }
    }
};

class Serializer {
public:
    static std::string serialize(const JsonValue& value) {
        std::ostringstream oss;
        serialize_value(value, oss);
        return oss.str();
    }

private:
    static void serialize_value(const JsonValue& value, std::ostringstream& oss) {
        switch (value.get_type()) {
            case JsonValue::NULL_TYPE:
                oss << "null";
                break;
            case JsonValue::BOOL:
                oss << (value.as_bool() ? "true" : "false");
                break;
            case JsonValue::INT:
                oss << value.as_int();
                break;
            case JsonValue::DOUBLE:
                oss << value.as_double();
                break;
            case JsonValue::STRING:
                serialize_string(value.as_string(), oss);
                break;
            case JsonValue::OBJECT:
                serialize_object(value.as_object(), oss);
                break;
            case JsonValue::ARRAY:
                serialize_array(value.as_array(), oss);
                break;
        }
    }

    static void serialize_string(const std::string& str, std::ostringstream& oss) {
        oss << '"';
        for (char c : str) {
            switch (c) {
                case '"': oss << '\\"'; break;
                case '\\': oss << '\\\\'; break;
                case '/': oss << '\\/'; break;
                case '\b': oss << '\\b'; break;
                case '\f': oss << '\\f'; break;
                case '\n': oss << '\\n'; break;
                case '\r': oss << '\\r'; break;
                case '\t': oss << '\\t'; break;
                default: oss << c; break;
            }
        }
        oss << '"';
    }

    static void serialize_object(const std::map<std::string, JsonValue>& obj, std::ostringstream& oss) {
        oss << '{';
        bool first = true;
        for (const auto& pair : obj) {
            if (!first) oss << ',';
            first = false;
            serialize_string(pair.first, oss);
            oss << ':';
            serialize_value(pair.second, oss);
        }
        oss << '}';
    }

    static void serialize_array(const std::vector<JsonValue>& arr, std::ostringstream& oss) {
        oss << '[';
        bool first = true;
        for (const auto& value : arr) {
            if (!first) oss << ',';
            first = false;
            serialize_value(value, oss);
        }
        oss << ']';
    }
};

} // namespace json