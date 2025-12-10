#pragma once
#include "Entity.h"
#include "Entitymanager.h"
#include "Components.h"
#include <cmath>

/**
 * @file CameraUtils.h
 * @brief Camera utility functions for editor operations
 * 
 * Provides helper functions for common camera operations in the editor,
 * such as focusing the camera on selected entities.
 */

// Camera utility functions for editor operations
namespace CameraUtils {
    /**
     * @brief Focus camera on a specific entity
     * 
     * Automatically positions and orients the active camera to view the target entity.
     * Calculates optimal camera distance based on entity scale and positions the camera
     * behind and above the entity for a good viewing angle.
     * 
     * @param entity The entity to focus on (must have TransformComponent)
     * @param em EntityManager to find the active camera
     */
    inline void focusOnEntity(Entity* entity, EntityManager& em) {
        if (!entity) return;
        
        // Find the main camera
        Entity* camera = em.getEntityByTag("MainCamera");
        if (!camera) {
            // Try to find any active camera
            auto cameras = em.getEntitiesWith<CameraComponent>();
            for (auto* cam : cameras) {
                auto* camComp = cam->getComponent<CameraComponent>();
                if (camComp && camComp->isActive) {
                    camera = cam;
                    break;
                }
            }
        }
        
        if (!camera) return;
        
        auto* camTransform = camera->getComponent<TransformComponent>();
        auto* entityTransform = entity->getComponent<TransformComponent>();
        
        if (!camTransform || !entityTransform) return;
        
        // Calculate appropriate camera distance based on entity scale
        glm::vec3 entityScale = entityTransform->scale;
        float maxScale = glm::max(glm::max(entityScale.x, entityScale.y), entityScale.z);
        float distance = maxScale * 3.0f; // Place camera 3x the max scale away
        
        // Minimum distance to ensure we don't get too close
        if (distance < 5.0f) distance = 5.0f;
        
        // Position camera behind and above the entity
        glm::vec3 offset = glm::vec3(0.0f, distance * 0.5f, distance);
        camTransform->position = entityTransform->position + offset;
        
        // Point camera at the entity by adjusting rotation
        glm::vec3 direction = glm::normalize(entityTransform->position - camTransform->position);
        float pitch = std::asin(-direction.y);
        float yaw = std::atan2(direction.x, direction.z);
        
        camTransform->rotation = glm::vec3(pitch, yaw, 0.0f);
        
        // Update camera component's direction vectors
        auto* camComp = camera->getComponent<CameraComponent>();
        if (camComp) {
            camComp->updateVectors(camTransform->rotation);
        }
    }
}
