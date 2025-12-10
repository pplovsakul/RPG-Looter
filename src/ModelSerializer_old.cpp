
#include "ModelSerializer.h"
#include "Components.h"
#include "JsonParser.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

static std::string escapeString(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
        case '\"': o << "\\\""; break;
        case '\\': o << "\\\\"; break;
        case '\b': o << "\\b"; break;
        case '\f': o << "\\f"; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
            } else o << c;
        }
    }
    return o.str();
}

bool ModelSerializer::saveModelToFile(const ModelComponent& model, const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) return false;

    out << "{\n  \"shapes\": [\n";
    bool first = true;
    for (const auto& s : model.shapes) {
        if (!first) out << ",\n";
        first = false;
        out << "    {\n";
        out << "      \"type\": \"" << (s.type == ModelComponent::ShapeType::Rectangle ? "Rectangle"
            : s.type == ModelComponent::ShapeType::Triangle ? "Triangle"
            : s.type == ModelComponent::ShapeType::Circle ? "Circle" : "TexturedQuad") << "\",\n";
        out << "      \"position\": [" << s.position.x << ", " << s.position.y << "],\n";
        out << "      \"rotation\": " << s.rotation << ",\n";
        out << "      \"size\": [" << s.size.x << ", " << s.size.y << "],\n";
        out << "      \"scale\": [" << s.scale.x << ", " << s.scale.y << "],\n";
        out << "      \"color\": [" << s.color.r << ", " << s.color.g << ", " << s.color.b << "],\n";
        out << "      \"filled\": " << (s.filled ? "true" : "false") << ",\n";
        out << "      \"layer\": " << s.layer << ",\n";
        out << "      \"texture\": \"" << escapeString(s.textureName) << "\"\n";
        out << "    }";
    }
    out << "\n  ]\n}\n";
    out.close();
    return true;
}

std::unique_ptr<ModelComponent> ModelSerializer::loadModelFromFile(const std::string& path) {
    try {
        JsonParser parser;
        JsonValue root = parser.parseFile(path);
        if (!root.isObject()) return nullptr;
        auto it = root.asObject().find("shapes");
        if (it == root.asObject().end() || !it->second.isArray()) return nullptr;

        auto model = std::make_unique<ModelComponent>();
        for (const auto& sv : it->second.asArray()) {
            if (!sv.isObject()) continue;
            const auto& so = sv.asObject();
            ModelComponent::Shape s;
            // type
            auto itType = so.find("type");
            if (itType != so.end() && itType->second.isString()) {
                std::string t = itType->second.asString();
                if (t == "Rectangle") s.type = ModelComponent::ShapeType::Rectangle;
                else if (t == "Triangle") s.type = ModelComponent::ShapeType::Triangle;
                else if (t == "Circle") s.type = ModelComponent::ShapeType::Circle;
                else s.type = ModelComponent::ShapeType::TexturedQuad;
            }
            auto itPos = so.find("position");
            if (itPos != so.end() && itPos->second.isArray()) {
                const auto& a = itPos->second.asArray();
                if (a.size() >= 2) { s.position.x = (float)a[0].asNumber(); s.position.y = (float)a[1].asNumber(); }
            }
            auto itRot = so.find("rotation");
            if (itRot != so.end() && itRot->second.isNumber()) s.rotation = (float)itRot->second.asNumber();
            auto itSize = so.find("size");
            if (itSize != so.end() && itSize->second.isArray()) {
                const auto& a = itSize->second.asArray();
                if (a.size() >= 2) { s.size.x = (float)a[0].asNumber(); s.size.y = (float)a[1].asNumber(); }
            }
            auto itScale = so.find("scale");
            if (itScale != so.end() && itScale->second.isArray()) {
                const auto& a = itScale->second.asArray();
                if (a.size() >= 2) { s.scale.x = (float)a[0].asNumber(); s.scale.y = (float)a[1].asNumber(); }
            }
            auto itColor = so.find("color");
            if (itColor != so.end() && itColor->second.isArray()) {
                const auto& a = itColor->second.asArray();
                if (a.size() >= 3) { s.color.r = (float)a[0].asNumber(); s.color.g = (float)a[1].asNumber(); s.color.b = (float)a[2].asNumber(); }
            }
            auto itFilled = so.find("filled");
            if (itFilled != so.end() && itFilled->second.isBool()) s.filled = itFilled->second.asBool();
            auto itLayer = so.find("layer");
            if (itLayer != so.end() && itLayer->second.isNumber()) s.layer = (int)itLayer->second.asNumber();
            auto itTex = so.find("texture");
            if (itTex != so.end() && itTex->second.isString()) s.textureName = itTex->second.asString();

            model->shapes.push_back(s);
        }
        return model;
    } catch (...) {
        return nullptr;
    }
}