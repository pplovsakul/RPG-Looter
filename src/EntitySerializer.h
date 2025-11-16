#pragma once
#include <string>

class EntityManager;

class EntitySerializer {
public:
    // Save all currently active entities (and supported components) to path
    static bool saveEntities(const EntityManager& em, const std::string& path);

    // Load entities from path (creates new entities in the manager)
    static bool loadEntities(EntityManager& em, const std::string& path);
};