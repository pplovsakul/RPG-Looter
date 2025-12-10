#include "PlayerControllerSystem.h"
#include "Components.h"
#include "Entitymanager.h"

PlayerControllerSystem::PlayerControllerSystem(InputSystem* input)
    : inputSystem(input) {
}

void PlayerControllerSystem::update(EntityManager& em, float deltaTime) {
    auto entities = em.getAllEntities();
    for (auto* entity : entities) {
        auto* controller = entity->getComponent<CharacterControllerComponent>();
        auto* transform = entity->getComponent<TransformComponent>();
        
        if (controller && transform) {
            processMovementInput(entity, controller, deltaTime);
            handleJump(controller);
            applyPhysics(transform, controller, deltaTime);
            checkGrounded(transform, controller);
        }
    }
}

void PlayerControllerSystem::processMovementInput(Entity* player, CharacterControllerComponent* controller, float deltaTime) {
    // Reset move input
    controller->moveInput = glm::vec2(0.0f);
    
    // Get input from InputSystem (would be better to use action system)
    // For now, assume direct key checks
    // This would integrate with your InputSystem's action mapping
    
    // Placeholder: moveInput should be set by input events
    // controller->moveInput.y = forward/backward input
    // controller->moveInput.x = left/right input
}

void PlayerControllerSystem::applyPhysics(TransformComponent* transform, CharacterControllerComponent* controller, float deltaTime) {
    // Apply movement
    float currentSpeed = controller->moveSpeed;
    if (controller->isSprinting) {
        currentSpeed *= controller->sprintMultiplier;
    }
    
    // Apply horizontal movement
    glm::vec3 movement(0.0f);
    movement.x = controller->moveInput.x * currentSpeed * deltaTime;
    movement.z = controller->moveInput.y * currentSpeed * deltaTime;
    
    // Apply gravity
    if (controller->useGravity && !controller->isGrounded) {
        controller->velocity.y += controller->gravity * deltaTime;
    } else if (controller->isGrounded && controller->velocity.y < 0) {
        controller->velocity.y = 0;
    }
    
    // Apply velocity
    movement += controller->velocity * deltaTime;
    
    // Update position
    transform->position += glm::vec2(movement.x, movement.z);
    
    // For 3D, we'd also update Y position if we had 3D transform
    // For now, keep it compatible with existing 2D system
}

void PlayerControllerSystem::checkGrounded(TransformComponent* transform, CharacterControllerComponent* controller) {
    // Simple ground check - would use raycast in full implementation
    // For now, assume grounded if Y velocity is near zero or position is at ground level
    controller->isGrounded = (transform->position.y <= 0.1f);
}

void PlayerControllerSystem::handleJump(CharacterControllerComponent* controller) {
    if (controller->wantsToJump && controller->isGrounded && !controller->isJumping) {
        controller->velocity.y = controller->jumpForce;
        controller->isJumping = true;
        controller->isGrounded = false;
    }
    
    if (!controller->wantsToJump) {
        controller->isJumping = false;
    }
}
