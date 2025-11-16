#pragma once

#include "System.h"
#include "Entitymanager.h"
#include "CollisionUtils.h"
#include "vendor/imgui/imgui.h"
#include <vector>

class CollisionSystem : public System {
public:
    CollisionSystem() = default;
    void update(EntityManager& em, float deltaTime) override;

    // utility: return list of pairs that are colliding this frame
    std::vector<std::pair<Entity*, Entity*>> getCollisions() const { return collisions; }

    // Check if moving entity `e` to `proposedPos` would cause a collision with any other entity
    bool wouldCollide(Entity* e, const glm::vec2& proposedPos, EntityManager& em) const;

private:
    std::vector<std::pair<Entity*, Entity*>> collisions;
};
