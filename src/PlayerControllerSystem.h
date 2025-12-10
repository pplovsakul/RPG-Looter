#pragma once

#include "System.h"
#include "InputSystem.h"
#include "Components.h"

class PlayerControllerSystem : public System {
private:
    InputSystem* inputSystem;

public:
    PlayerControllerSystem(InputSystem* input);
    
    void update(EntityManager& em, float deltaTime) override;

private:
    void processMovementInput(Entity* player, CharacterControllerComponent* controller, float deltaTime);
    void applyPhysics(TransformComponent* transform, CharacterControllerComponent* controller, float deltaTime);
    void checkGrounded(TransformComponent* transform, CharacterControllerComponent* controller);
    void handleJump(CharacterControllerComponent* controller);
};
