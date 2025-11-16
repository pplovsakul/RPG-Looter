#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>

class JsonValue {
public:
    using Object = std::unordered_map<std::string, JsonValue>;
    using Array = std::vector<JsonValue>;

    std::variant<std::string, double, bool, Object, Array> value;

    bool isString() const { return std::holds_alternative<std::string>(value); }
    bool isNumber() const { return std::holds_alternative<double>(value); }
    bool isBool() const { return std::holds_alternative<bool>(value); }
    bool isObject() const { return std::holds_alternative<Object>(value); }
    bool isArray() const { return std::holds_alternative<Array>(value); }

    const std::string& asString() const { return std::get<std::string>(value); }
    double asNumber() const { return std::get<double>(value); }
    bool asBool() const { return std::get<bool>(value); }
    const Object& asObject() const { return std::get<Object>(value); }
    const Array& asArray() const { return std::get<Array>(value); }
};

class JsonParser {
    std::string text;
    size_t pos = 0;

    void skipWhitespace() {
        while (pos < text.size() && std::isspace(text[pos])) pos++;
    }

    char peek() const { return text[pos]; }
    char get() { return text[pos++]; }

    JsonValue parseValue() {
        skipWhitespace();
        if (peek() == '{') return parseObject();
        if (peek() == '[') return parseArray();
        if (peek() == '"') return parseString();
        if (std::isdigit(peek()) || peek() == '-') return parseNumber();
        if (text.compare(pos, 4, "true") == 0) { pos += 4; return JsonValue{ true }; }
        if (text.compare(pos, 5, "false") == 0) { pos += 5; return JsonValue{ false }; }
        throw std::runtime_error("Unexpected JSON value");
    }

    JsonValue parseString() {
        get(); // skip "
        std::string result;
        while (pos < text.size() && peek() != '"') {
            result += get();
        }
        get(); // skip "
        return JsonValue{ result };
    }

    JsonValue parseNumber() {
        std::string numStr;
        if (peek() == '-') numStr += get();
        while (pos < text.size() && (std::isdigit(peek()) || peek() == '.')) {
            numStr += get();
        }
        return JsonValue{ std::stod(numStr) };
    }

    JsonValue parseArray() {
        get(); // skip [
        JsonValue::Array arr;
        skipWhitespace();
        while (peek() != ']') {
            arr.push_back(parseValue());
            skipWhitespace();
            if (peek() == ',') get();
            skipWhitespace();
        }
        get(); // skip ]
        return JsonValue{ arr };
    }

    JsonValue parseObject() {
        get(); // skip {
        JsonValue::Object obj;
        skipWhitespace();
        while (peek() != '}') {
            auto key = parseString().asString();
            skipWhitespace();
            if (get() != ':') throw std::runtime_error("Expected ':' after key");
            skipWhitespace();
            obj[key] = parseValue();
            skipWhitespace();
            if (peek() == ',') get();
            skipWhitespace();
        }
        get(); // skip }
        return JsonValue{ obj };
    }

public:
    JsonValue parse(const std::string& input) {
        text = input;
        pos = 0;
        return parseValue();
    }

    JsonValue parseFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) throw std::runtime_error("Failed to open JSON file: " + path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return parse(buffer.str());
    }
};