#include "EntitySerializer.h"
#include "Entitymanager.h"
#include "Entity.h"
#include "Components.h"
#include "JsonParser.h"
#include <fstream>
#include <sstream>
#include <iomanip>

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

bool EntitySerializer::saveEntities(const EntityManager& em, const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) return false;

    out << "{\n  \"entities\": [\n";

    auto entities = em.getAllEntities();
    bool firstE = true;
    for (auto* e : entities) {
        if (!firstE) out << ",\n";
        firstE = false;

        out << "    {\n";
        out << "      \"id\": " << e->id << ",\n";
        out << "      \"tag\": \"" << escapeString(e->tag) << "\",\n";
        out << "      \"components\": {\n";

        bool firstComp = true;

        if (e->hasComponent<TransformComponent>()) {
            if (!firstComp) out << ",\n";
            firstComp = false;
            auto* t = e->getComponent<TransformComponent>();
            out << "        \"Transform\": {\n";
            out << "          \"position\": [" << t->position.x << ", " << t->position.y << ", " << t->position.z << "],\n";
            out << "          \"rotation\": [" << t->rotation.x << ", " << t->rotation.y << ", " << t->rotation.z << "],\n";
            out << "          \"scale\": [" << t->scale.x << ", " << t->scale.y << ", " << t->scale.z << "]\n";
            out << "        }";
        }

        if (e->hasComponent<RenderComponent>()) {
            if (!firstComp) out << ",\n";
            firstComp = false;
            auto* r = e->getComponent<RenderComponent>();
            out << "        \"Render\": {\n";
            out << "          \"meshName\": \"" << escapeString(r->meshName) << "\",\n";
            out << "          \"shaderName\": \"" << escapeString(r->shaderName) << "\",\n";
            out << "          \"textureName\": \"" << escapeString(r->textureName) << "\",\n";
            out << "          \"color\": [" << r->color.r << ", " << r->color.g << ", " << r->color.b << "],\n";
            out << "          \"alpha\": " << r->alpha << ",\n";
            out << "          \"enabled\": " << (r->enabled ? "true" : "false") << ",\n";
            out << "          \"renderLayer\": " << r->renderLayer << "\n";
            out << "        }";
        }

        if (e->hasComponent<AudioComponent>()) {
            if (!firstComp) out << ",\n";
            firstComp = false;
            auto* a = e->getComponent<AudioComponent>();
            out << "        \"Audio\": {\n";
            out << "          \"soundName\": \"" << escapeString(a->soundName) << "\",\n";
            out << "          \"playOnce\": " << (a->playOnce ? "true" : "false") << ",\n";
            out << "          \"loop\": " << (a->loop ? "true" : "false") << ",\n";
            out << "          \"volume\": " << a->volume << ",\n";
            out << "          \"pitch\": " << a->pitch << "\n";
            out << "        }";
        }



        out << "\n      }\n    }";
    }

    out << "\n  ]\n}\n";
    out.close();
    return true;
}

bool EntitySerializer::loadEntities(EntityManager& em, const std::string& path) {
    try {
        JsonParser parser;
        JsonValue root = parser.parseFile(path);
        if (!root.isObject()) return false;
        const auto& obj = root.asObject();
        auto it = obj.find("entities");
        if (it == obj.end() || !it->second.isArray()) return false;
        const auto& arr = it->second.asArray();

        for (const auto& entVal : arr) {
            if (!entVal.isObject()) continue;
            const auto& entObj = entVal.asObject();

            std::string tag = "";
            auto itTag = entObj.find("tag");
            if (itTag != entObj.end() && itTag->second.isString()) tag = itTag->second.asString();

            Entity* e = em.createEntity();
            e->tag = tag;

            auto itComps = entObj.find("components");
            if (itComps == entObj.end() || !itComps->second.isObject()) continue;
            const auto& comps = itComps->second.asObject();

            // Transform
            auto itT = comps.find("Transform");
            if (itT != comps.end() && itT->second.isObject()) {
                const auto& to = itT->second.asObject();
                auto* tc = e->addComponent<TransformComponent>();
                auto itPos = to.find("position");
                if (itPos != to.end() && itPos->second.isArray()) {
                    const auto& a = itPos->second.asArray();
                    if (a.size() >= 2) {
                        tc->position.x = a[0].asNumber();
                        tc->position.y = a[1].asNumber();
                    }
                    if (a.size() >= 3) {
                        tc->position.z = a[2].asNumber();
                    }
                }
                auto itRot = to.find("rotation");
                if (itRot != to.end()) {
                    if (itRot->second.isNumber()) {
                        // Old format: single number
                        tc->rotation.y = (float)itRot->second.asNumber();
                    } else if (itRot->second.isArray()) {
                        // New format: array
                        const auto& a = itRot->second.asArray();
                        if (a.size() >= 1) tc->rotation.x = a[0].asNumber();
                        if (a.size() >= 2) tc->rotation.y = a[1].asNumber();
                        if (a.size() >= 3) tc->rotation.z = a[2].asNumber();
                    }
                }
                auto itScale = to.find("scale");
                if (itScale != to.end() && itScale->second.isArray()) {
                    const auto& a = itScale->second.asArray();
                    if (a.size() >= 2) {
                        tc->scale.x = a[0].asNumber();
                        tc->scale.y = a[1].asNumber();
                    }
                    if (a.size() >= 3) {
                        tc->scale.z = a[2].asNumber();
                    }
                }
            }

            // Render
            auto itR = comps.find("Render");
            if (itR != comps.end() && itR->second.isObject()) {
                const auto& ro = itR->second.asObject();
                auto* rc = e->addComponent<RenderComponent>();
                auto itMesh = ro.find("meshName");
                if (itMesh != ro.end() && itMesh->second.isString()) rc->meshName = itMesh->second.asString();
                auto itShader = ro.find("shaderName");
                if (itShader != ro.end() && itShader->second.isString()) rc->shaderName = itShader->second.asString();
                auto itTex = ro.find("textureName");
                if (itTex != ro.end() && itTex->second.isString()) rc->textureName = itTex->second.asString();
                auto itColor = ro.find("color");
                if (itColor != ro.end() && itColor->second.isArray()) {
                    const auto& a = itColor->second.asArray();
                    if (a.size() >= 3) {
                        rc->color.r = (float)a[0].asNumber();
                        rc->color.g = (float)a[1].asNumber();
                        rc->color.b = (float)a[2].asNumber();
                    }
                }
                auto itAlpha = ro.find("alpha");
                if (itAlpha != ro.end() && itAlpha->second.isNumber()) rc->alpha = (float)itAlpha->second.asNumber();
                auto itEnabled = ro.find("enabled");
                if (itEnabled != ro.end() && itEnabled->second.isBool()) rc->enabled = itEnabled->second.asBool();
                auto itLayer = ro.find("renderLayer");
                if (itLayer != ro.end() && itLayer->second.isNumber()) rc->renderLayer = (int)itLayer->second.asNumber();
            }

            // Audio
            auto itA = comps.find("Audio");
            if (itA != comps.end() && itA->second.isObject()) {
                const auto& ao = itA->second.asObject();
                auto* ac = e->addComponent<AudioComponent>();
                auto itName = ao.find("soundName");
                if (itName != ao.end() && itName->second.isString()) ac->soundName = itName->second.asString();
                auto itPlay = ao.find("playOnce");
                if (itPlay != ao.end() && itPlay->second.isBool()) ac->playOnce = itPlay->second.asBool();
                auto itLoop = ao.find("loop");
                if (itLoop != ao.end() && itLoop->second.isBool()) ac->loop = itLoop->second.asBool();
                auto itVol = ao.find("volume");
                if (itVol != ao.end() && itVol->second.isNumber()) ac->volume = (float)itVol->second.asNumber();
                auto itPitch = ao.find("pitch");
                if (itPitch != ao.end() && itPitch->second.isNumber()) ac->pitch = (float)itPitch->second.asNumber();
            }


        }

        return true;
    } catch (...) {
        return false;
    }
}