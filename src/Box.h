#pragma once
#include "Ray.h"
#include "Hit.h"
#include "Material.h"
#include "vendor/glm/glm.hpp"

// Axis-Aligned Bounding Box (AABB) for ray tracing
class Box {
public:
    glm::vec3 minBounds; // Minimum corner of the box
    glm::vec3 maxBounds; // Maximum corner of the box
    Material material;

    Box(const glm::vec3& min, const glm::vec3& max, const Material& mat = Material()) 
        : minBounds(min), maxBounds(max), material(mat) {}

    // Convenience constructor: center and size
    static Box fromCenterSize(const glm::vec3& center, const glm::vec3& size, const Material& mat = Material()) {
        glm::vec3 halfSize = size * 0.5f;
        return Box(center - halfSize, center + halfSize, mat);
    }

    // Ray-Box intersection using slab method
    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
        float t0 = tMin;
        float t1 = tMax;

        // For each axis (x, y, z)
        for (int axis = 0; axis < 3; ++axis) {
            float invD = 1.0f / r.direction[axis];
            float tNear = (minBounds[axis] - r.origin[axis]) * invD;
            float tFar = (maxBounds[axis] - r.origin[axis]) * invD;

            // Swap if needed (ray direction is negative)
            if (tNear > tFar) {
                float temp = tNear;
                tNear = tFar;
                tFar = temp;
            }

            // Update intersection interval
            t0 = tNear > t0 ? tNear : t0;
            t1 = tFar < t1 ? tFar : t1;

            // No intersection if interval is invalid
            if (t0 > t1) return false;
        }

        // We have an intersection at t0 (entry point)
        rec.t = t0;
        rec.point = r.at(rec.t);

        // Calculate normal based on which face was hit
        glm::vec3 center = (minBounds + maxBounds) * 0.5f;
        glm::vec3 localHit = rec.point - center;
        glm::vec3 halfSize = (maxBounds - minBounds) * 0.5f;

        // Find which axis has the largest normalized hit
        glm::vec3 normal(0.0f);
        float maxComponent = 0.0f;
        for (int i = 0; i < 3; ++i) {
            float component = fabsf(localHit[i] / halfSize[i]);
            if (component > maxComponent) {
                maxComponent = component;
                normal = glm::vec3(0.0f);
                normal[i] = localHit[i] > 0.0f ? 1.0f : -1.0f;
            }
        }

        rec.setFaceNormal(r, normal);
        rec.material = material;
        return true;
    }
};
