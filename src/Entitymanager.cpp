#include "EntityManager.h"
#include "Components.h"

Entity* EntityManager::createEntity() {
    auto entity = std::make_unique<Entity>(nextID++);
    Entity* ptr = entity.get();
    entities.push_back(std::move(entity));
    return ptr;
}

void EntityManager::destroyEntity(unsigned int id) {
    for (auto& entity : entities) {
        if (entity->id == id) {
            entity->active = false;
            break;
        }
    }
}

void EntityManager::cleanup() {
    // Entities entfernen (Textures werden vom AssetManager verwaltet)
    // Kein manueller Texture-Delete nötig da RenderComponent nur textureName (String) hat
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](const std::unique_ptr<Entity>& e) { return !e->active; }),
        entities.end()
    );
}

std::vector<Entity*> EntityManager::getAllEntities() const {
    std::vector<Entity*> result;
    for (auto& entity : entities) {
        if (entity->active) {
            result.push_back(entity.get());
        }
    }
    return result;
}

Entity* EntityManager::getEntityByTag(const std::string& tag) {
    for (auto& entity : entities) {
        if (entity->active && entity->tag == tag) {
            return entity.get();
        }
    }
    return nullptr;
}

Entity* EntityManager::getEntityByID(unsigned int id) {
    for (auto& entity : entities) {
        if (entity->active && entity->id == id) {
            return entity.get();
        }
    }
    return nullptr;
}
