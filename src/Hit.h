#pragma once
#include "vendor/glm/glm.hpp"

// Forward declaration to avoid circular dependency
struct Material;

struct HitRecord {
    float t = 0.0f;          // Parameter along ray
    glm::vec3 point;         // Intersection point
    glm::vec3 normal;        // Surface normal at hit
    bool frontFace = true;   // true if ray hits the front face
    Material material;       // Material at hit point

    inline void setFaceNormal(const Ray& r, const glm::vec3& outwardNormal) {
        frontFace = glm::dot(r.direction, outwardNormal) < 0.0f;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};