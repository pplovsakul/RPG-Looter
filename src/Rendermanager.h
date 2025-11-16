#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "Renderer.h"
#include "Shader.h"

class RenderManager {
public:
    RenderManager(Renderer& renderer, Shader& shader, const glm::mat4& proj)
        : renderer(renderer), shader(shader), proj(proj) {
    }

    // Objekt hinzufügen
    void AddObject(const std::string& name, std::unique_ptr<RenderObject> obj) {
        objects[name] = std::move(obj);
    }

    // Objekt holenm
    RenderObject* GetObject(const std::string& name) {
        auto it = objects.find(name);
        return (it != objects.end()) ? it->second.get() : nullptr;
    }

    void UpdateObject(const std::string& name, const glm::vec2& pos) {
        if (objects.find(name) != objects.end()) {
            objects[name]->UpdateModelMatrix(pos);
        }
    }

    // Zeichnet alle
    void DrawAll() {
        for (auto& [name, obj] : objects) {
            if (!obj->isVisible())  
                continue;
            renderer.Draw_RenderObject(*obj, shader, proj, obj->position);
        }
    }

private:
    Renderer& renderer;
    Shader& shader;
    glm::mat4 proj;
    std::unordered_map<std::string, std::unique_ptr<RenderObject>> objects;
};