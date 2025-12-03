#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cctype>

class JsonValue;

class JsonObject {
public:
    JsonObject() = default;

    JsonValue& operator[](const std::string& key);
    const JsonValue& operator[](const std::string& key) const;

    bool has(const std::string& key) const;
    void set(const std::string& key, const JsonValue& value);

    std::string toString() const;

private:
    std::map<std::string, JsonValue> values;
};

class JsonArray {
public:
    JsonArray() = default;

    JsonValue& operator[](size_t index);
    const JsonValue& operator[](size_t index) const;

    void push_back(const JsonValue& value);
    size_t size() const;

    std::string toString() const;

private:
    std::vector<JsonValue> values;
};

class JsonValue {
public:
    JsonValue() : type(Type::Null) {}
    JsonValue(const char* str) : type(Type::String), strValue(str) {}
    JsonValue(const std::string& str) : type(Type::String), strValue(str) {}
    JsonValue(int num) : type(Type::Int), intValue(num) {}
    JsonValue(double num) : type(Type::Double), doubleValue(num) {}
    JsonValue(bool b) : type(Type::Bool), boolValue(b) {}
    JsonValue(const JsonObject& obj) : type(Type::Object), objValue(obj) {}
    JsonValue(const JsonArray& arr) : type(Type::Array), arrValue(arr) {}

    enum class Type { Null, String, Int, Double, Bool, Object, Array };

    Type getType() const { return type; }

    const std::string& asString() const {
        if (type != Type::String) throw std::runtime_error("Not a string");
        return strValue;
    }

    int asInt() const {
        if (type != Type::Int) throw std::runtime_error("Not an integer");
        return intValue;
    }

    double asDouble() const {
        if (type != Type::Double) throw std::runtime_error("Not a double");
        return doubleValue;
    }

    bool asBool() const {
        if (type != Type::Bool) throw std::runtime_error("Not a boolean");
        return boolValue;
    }

    const JsonObject& asObject() const {
        if (type != Type::Object) throw std::runtime_error("Not an object");
        return objValue;
    }

    const JsonArray& asArray() const {
        if (type != Type::Array) throw std::runtime_error("Not an array");
        return arrValue;
    }

    std::string toString() const;

private:
    Type type;
    std::string strValue;
    int intValue = 0;
    double doubleValue = 0.0;
    bool boolValue = false;
    JsonObject objValue;
    JsonArray arrValue;

    friend class JsonObject;
    friend class JsonArray;
};

inline JsonValue& JsonObject::operator[](const std::string& key) {
    return values[key];
}

inline const JsonValue& JsonObject::operator[](const std::string& key) const {
    auto it = values.find(key);
    if (it == values.end()) throw std::runtime_error("Key not found: " + key);
    return it->second;
}

inline bool JsonObject::has(const std::string& key) const {
    return values.find(key) != values.end();
}

inline void JsonObject::set(const std::string& key, const JsonValue& value) {
    values[key] = value;
}

inline std::string JsonObject::toString() const {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& pair : values) {
        if (!first) oss << ",";
        oss << '"' << pair.first << '":' << pair.second.toString();
        first = false;
    }
    oss << "}";
    return oss.str();
}

inline JsonValue& JsonArray::operator[](size_t index) {
    return values[index];
}

inline const JsonValue& JsonArray::operator[](size_t index) const {
    return values[index];
}

inline void JsonArray::push_back(const JsonValue& value) {
    values.push_back(value);
}

inline size_t JsonArray::size() const {
    return values.size();
}

inline std::string JsonArray::toString() const {
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (const auto& value : values) {
        if (!first) oss << ",";
        oss << value.toString();
        first = false;
    }
    oss << "]";
    return oss.str();
}

inline std::string JsonValue::toString() const {
    switch (type) {
        case Type::Null: return "null";
        case Type::String: return '"' + strValue + '"';
        case Type::Int: return std::to_string(intValue);
        case Type::Double: return std::to_string(doubleValue);
        case Type::Bool: return boolValue ? "true" : "false";
        case Type::Object: return objValue.toString();
        case Type::Array: return arrValue.toString();
        default: return "null";
    }
}

inline JsonValue parseJson(const std::string& jsonStr);
