#pragma once
#include "Ray.h"
#include "Hit.h"
#include "Material.h"
#include "vendor/glm/glm.hpp"

class Sphere {
public:
    glm::vec3 center;
    float radius;
    Material material;

    Sphere(const glm::vec3& c, float r, const Material& mat = Material()) 
        : center(c), radius(r), material(mat) {}

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
        glm::vec3 oc = r.origin - center;
        float a = glm::dot(r.direction, r.direction);
        float half_b = glm::dot(oc, r.direction);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = half_b * half_b - a * c;
        if (discriminant < 0.0f) return false;
        float sqrtD = sqrtf(discriminant);

        // Find nearest root in [tMin, tMax]
        float root = (-half_b - sqrtD) / a;
        if (root < tMin || root > tMax) {
            root = (-half_b + sqrtD) / a;
            if (root < tMin || root > tMax) return false;
        }

        rec.t = root;
        rec.point = r.at(rec.t);
        glm::vec3 outwardNormal = (rec.point - center) / radius;
        rec.setFaceNormal(r, outwardNormal);
        rec.material = material;
        return true;
    }
};