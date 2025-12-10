#pragma once
#include <vector>
#include "vendor/glm/glm.hpp"
#include "Ray.h"
#include "Hit.h"
#include "Sphere.h"
#include "Box.h"
#include "Camera.h"

struct RTColor { float r, g, b; };

class RayTracer {
public:
    int width = 4;
    int height = 3;
    Camera camera;
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -0.5f));

    std::vector<Sphere> spheres;
    std::vector<Box> boxes;

    RayTracer(int w, int h) : width(w), height(h) {
        camera.aspect = float(w) / float(h);
        camera.update();
        // Default scene: ground sphere
        spheres.emplace_back(glm::vec3(0, -100.5f, 0), 100.0f); // ground
    }

    RTColor background(const Ray& r) const {
        // Sky gradient: blend between white and blue based on ray direction Y
        glm::vec3 unitDir = glm::normalize(r.direction);
        float t = 0.5f * (unitDir.y + 1.0f);
        glm::vec3 col = (1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
        return { col.r, col.g, col.b };
    }

    RTColor shade(const Ray& r) const {
        HitRecord closestRec;
        bool hitAnything = false;
        float closestSoFar = 1e30f;

        // Test spheres
        for (const auto& s : spheres) {
            HitRecord rec;
            if (s.hit(r, 0.001f, closestSoFar, rec)) {
                hitAnything = true;
                closestSoFar = rec.t;
                closestRec = rec;
            }
        }

        // Test boxes (AABBs)
        for (const auto& b : boxes) {
            HitRecord rec;
            if (b.hit(r, 0.001f, closestSoFar, rec)) {
                hitAnything = true;
                closestSoFar = rec.t;
                closestRec = rec;
            }
        }

        if (!hitAnything) return background(r);

        // Simple Lambert shading with a single directional light
        float NdotL = glm::max(0.0f, glm::dot(closestRec.normal, -lightDir));
        glm::vec3 baseColor = closestRec.frontFace ? glm::vec3(0.8f, 0.3f, 0.3f) : glm::vec3(0.3f, 0.8f, 0.3f);
        glm::vec3 col = (0.1f * baseColor) + (0.9f * baseColor * NdotL);
        return { col.r, col.g, col.b };
    }

    // Returns RGBA8 buffer
    std::vector<uint32_t> render() {
        camera.update();
        std::vector<uint32_t> pixels(width * height);
        for (int y = 0; y < height; ++y) {
            float v = float(y) / float(height - 1);
            for (int x = 0; x < width; ++x) {
                float u = float(x) / float(width - 1);
                Ray r = camera.getRay(u, 1.0f - v); // flip Y to match GL screen coords
                RTColor c = shade(r);
                // Clamp and pack to RGBA8
                auto to8 = [](float f) { return (uint8_t)(glm::clamp(f, 0.0f, 1.0f) * 255.0f + 0.5f); };
                uint32_t R = to8(c.r), G = to8(c.g), B = to8(c.b), A = 255;
                pixels[y * width + x] = (A << 24) | (B << 16) | (G << 8) | (R);
            }
        }
        return pixels;
    }
};