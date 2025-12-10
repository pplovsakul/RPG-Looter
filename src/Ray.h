#pragma once
#include "vendor/glm/glm.hpp"

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray() {}
    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(glm::normalize(direction)) {
    }

    // Punkt entlang des Strahls bei Parameter t
    glm::vec3 at(float t) const {
        return origin + t * direction;
    }
};