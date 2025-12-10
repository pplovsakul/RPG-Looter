#pragma once
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"
#include "Ray.h"

// Simple camera with FOV, aspect, position, and look-at
class Camera {
public:
    glm::vec3 position{ 0,0,3 };
    glm::vec3 target{ 0,0,0 };
    glm::vec3 up{ 0,1,0 };
    float vfov = 60.0f; // degrees
    float aspect = 16.0f / 9.0f;

    // Precomputed basis
    glm::vec3 u, v, w;
    glm::vec3 origin;
    glm::vec3 lowerLeftCorner;
    glm::vec3 horizontal;
    glm::vec3 vertical;

    void update() {
        origin = position;
        w = glm::normalize(position - target);
        u = glm::normalize(glm::cross(up, w));
        v = glm::cross(w, u);

        float theta = glm::radians(vfov);
        float h = tanf(theta / 2.0f);
        float viewportHeight = 2.0f * h;
        float viewportWidth = aspect * viewportHeight;

        horizontal = viewportWidth * u;
        vertical = viewportHeight * v;
        lowerLeftCorner = origin - horizontal * 0.5f - vertical * 0.5f - w;
    }

    Ray getRay(float s, float t) const {
        return Ray(origin, lowerLeftCorner + s * horizontal + t * vertical - origin);
    }
};