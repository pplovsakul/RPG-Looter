#pragma once
#include "vendor/glm/glm.hpp"
#include "Material.h"

// Triangle: Grundlegendes Dreieck-Primitiv für Mesh-Rendering
struct Triangle {
    glm::vec3 v0, v1, v2;  // Vertices
    glm::vec3 normal;      // Vorkompilierte Normale
    int materialIndex;
    
    Triangle() : v0(0.0f), v1(0.0f), v2(0.0f), normal(0.0f, 1.0f, 0.0f), materialIndex(0) {}
    
    Triangle(const glm::vec3& vertex0, const glm::vec3& vertex1, const glm::vec3& vertex2, int matIdx = 0)
        : v0(vertex0), v1(vertex1), v2(vertex2), materialIndex(matIdx) {
        // Berechne Normale (Counter-Clockwise Winding)
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        normal = glm::normalize(glm::cross(edge1, edge2));
    }
    
    // Hilfsfunktion: AABB für BVH
    void getBounds(glm::vec3& minBounds, glm::vec3& maxBounds) const {
        minBounds = glm::min(glm::min(v0, v1), v2);
        maxBounds = glm::max(glm::max(v0, v1), v2);
    }
    
    // Schwerpunkt für BVH-Konstruktion
    glm::vec3 centroid() const {
        return (v0 + v1 + v2) / 3.0f;
    }
};

// Mesh-Generierungs-Funktionen
namespace MeshGenerator {
    
    // Box als 12 Dreiecke (2 pro Seite)
    inline std::vector<Triangle> createBox(const glm::vec3& center, const glm::vec3& size, int materialIndex) {
        std::vector<Triangle> triangles;
        glm::vec3 halfSize = size * 0.5f;
        
        // 8 Vertices der Box
        glm::vec3 vertices[8] = {
            center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z), // 0
            center + glm::vec3( halfSize.x, -halfSize.y, -halfSize.z), // 1
            center + glm::vec3( halfSize.x,  halfSize.y, -halfSize.z), // 2
            center + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z), // 3
            center + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z), // 4
            center + glm::vec3( halfSize.x, -halfSize.y,  halfSize.z), // 5
            center + glm::vec3( halfSize.x,  halfSize.y,  halfSize.z), // 6
            center + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z)  // 7
        };
        
        // 12 Dreiecke (2 pro Seite, CCW Winding)
        // Front (+Z)
        triangles.emplace_back(vertices[4], vertices[5], vertices[6], materialIndex);
        triangles.emplace_back(vertices[4], vertices[6], vertices[7], materialIndex);
        // Back (-Z)
        triangles.emplace_back(vertices[1], vertices[0], vertices[3], materialIndex);
        triangles.emplace_back(vertices[1], vertices[3], vertices[2], materialIndex);
        // Right (+X)
        triangles.emplace_back(vertices[5], vertices[1], vertices[2], materialIndex);
        triangles.emplace_back(vertices[5], vertices[2], vertices[6], materialIndex);
        // Left (-X)
        triangles.emplace_back(vertices[0], vertices[4], vertices[7], materialIndex);
        triangles.emplace_back(vertices[0], vertices[7], vertices[3], materialIndex);
        // Top (+Y)
        triangles.emplace_back(vertices[3], vertices[7], vertices[6], materialIndex);
        triangles.emplace_back(vertices[3], vertices[6], vertices[2], materialIndex);
        // Bottom (-Y)
        triangles.emplace_back(vertices[0], vertices[1], vertices[5], materialIndex);
        triangles.emplace_back(vertices[0], vertices[5], vertices[4], materialIndex);
        
        return triangles;
    }
    
    // Icosphere - Subdivision einer Icosahedron für glatte Kugel
    inline std::vector<Triangle> createIcosphere(const glm::vec3& center, float radius, int subdivisions, int materialIndex) {
        std::vector<glm::vec3> vertices;
        std::vector<Triangle> triangles;
        
        // Golden Ratio
        const float t = (1.0f + sqrtf(5.0f)) / 2.0f;
        const float s = sqrtf(1.0f + t * t);
        
        // 12 Vertices des Icosahedron (normalisiert)
        vertices.push_back(glm::normalize(glm::vec3(-1, t, 0)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( 1, t, 0)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3(-1,-t, 0)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( 1,-t, 0)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( 0,-1, t)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( 0, 1, t)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( 0,-1,-t)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( 0, 1,-t)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( t, 0,-1)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3( t, 0, 1)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3(-t, 0,-1)) * radius + center);
        vertices.push_back(glm::normalize(glm::vec3(-t, 0, 1)) * radius + center);
        
        // 20 Faces des Icosahedron
        std::vector<glm::ivec3> faces = {
            {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
            {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
            {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
            {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
        };
        
        // Subdivision (optional - 0 subdivisions = 20 triangles, 1 = 80, 2 = 320, etc.)
        for (int sub = 0; sub < subdivisions; ++sub) {
            std::vector<glm::ivec3> newFaces;
            for (const auto& face : faces) {
                // Mittelp

unkte der Kanten
                glm::vec3 mid01 = glm::normalize((vertices[face.x] - center + vertices[face.y] - center) * 0.5f) * radius + center;
                glm::vec3 mid12 = glm::normalize((vertices[face.y] - center + vertices[face.z] - center) * 0.5f) * radius + center;
                glm::vec3 mid20 = glm::normalize((vertices[face.z] - center + vertices[face.x] - center) * 0.5f) * radius + center;
                
                int idx01 = vertices.size(); vertices.push_back(mid01);
                int idx12 = vertices.size(); vertices.push_back(mid12);
                int idx20 = vertices.size(); vertices.push_back(mid20);
                
                // 4 neue Dreiecke
                newFaces.push_back({face.x, idx01, idx20});
                newFaces.push_back({face.y, idx12, idx01});
                newFaces.push_back({face.z, idx20, idx12});
                newFaces.push_back({idx01, idx12, idx20});
            }
            faces = newFaces;
        }
        
        // Konvertiere zu Triangles
        for (const auto& face : faces) {
            triangles.emplace_back(vertices[face.x], vertices[face.y], vertices[face.z], materialIndex);
        }
        
        return triangles;
    }
}
