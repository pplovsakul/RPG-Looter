#pragma once
#include "Entity.h"
#include <vector>
#include <memory>
#include <algorithm>

class EntityManager {
private:
    std::vector<std::unique_ptr<Entity>> entities;
    unsigned int nextID = 0;

public:
    Entity* createEntity();
    void destroyEntity(unsigned int id);
    void cleanup();

    Entity* getEntityByTag(const std::string& tag);
	Entity* getEntityByID(unsigned int id);

    template<typename... Components>
    std::vector<Entity*> getEntitiesWith() const;

    std::vector<Entity*> getAllEntities() const;
};

template<typename... Components>
std::vector<Entity*> EntityManager::getEntitiesWith() const {
    std::vector<Entity*> result;
    // ✅ OPTIMIZATION: Reserve space to avoid reallocation
    result.reserve(entities.size() / 2); // Heuristic: assume ~50% match
    for (auto& entity : entities) {
        if (entity->active && (entity->hasComponent<Components>() && ...)) {
            result.push_back(entity.get());
        }
    }
    return result;
}