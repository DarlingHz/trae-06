#pragma once
#include <string>
#include <sstream>
#include <vector>

class JsonUtils {
public:
    static std::string escapeString(const std::string& str) {
        std::stringstream ss;
        for (char c : str) {
            switch (c) {
                case '"': ss << '\\' << '"'; break;
                case '\\': ss << '\\' << '\\'; break;
                case '\n': ss << '\\' << 'n'; break;
                case '\t': ss << '\\' << 't'; break;
                case '\r': ss << '\\' << 'r'; break;
                case '\b': ss << '\\' << 'b'; break;
                case '\f': ss << '\\' << 'f'; break;
                default: ss << c; break;
            }
        }
        return ss.str();
    }

    static std::string ToJson(const std::string& key, const std::string& value) {
        std::stringstream ss;
        ss << '"' << escapeString(key) << '":"' << escapeString(value) << '"';
        return ss.str();
    }

    static std::string ToJson(const std::string& key, int value) {
        std::stringstream ss;
        ss << '"' << escapeString(key) << '":' << value;
        return ss.str();
    }

    static std::string ToJson(const std::string& key, double value) {
        std::stringstream ss;
        ss << '"' << escapeString(key) << '":' << value;
        return ss.str();
    }

    static std::string ToJson(const std::string& key, bool value) {
        std::stringstream ss;
        ss << '"' << escapeString(key) << '":' << (value ? "true" : "false");
        return ss.str();
    }

    static std::string ToJson(const std::string& key, const std::vector<std::string>& values) {
        std::stringstream ss;
        ss << '"' << escapeString(key) << '":[';
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) ss << ',';
            ss << '"' << escapeString(values[i]) << '"';
        }
        ss << ']';
        return ss.str();
    }
};
