#pragma once
#include "Entitymanager.h"

class System {
public:

    virtual ~System() = default;
    virtual void update(EntityManager& em, float deltaTime) = 0;
};