#pragma once

#include "vendor/glm/glm.hpp"
#include <cmath>

/**
 * Triangle - Dreieck-Struktur für Kollisionserkennung
 * 
 * Speichert die drei Eckpunkte eines Dreiecks und bietet
 * Hilfsfunktionen für geometrische Berechnungen.
 */
struct Triangle
{
    glm::vec3 v0, v1, v2;  // Die drei Eckpunkte

    Triangle() = default;

    Triangle(const glm::vec3& vertex0, const glm::vec3& vertex1, const glm::vec3& vertex2)
        : v0(vertex0), v1(vertex1), v2(vertex2)
    {
    }

    /**
     * Berechnet die Normale des Dreiecks (nicht normalisiert)
     */
    glm::vec3 GetNormal() const
    {
        return glm::cross(v1 - v0, v2 - v0);
    }

    /**
     * Berechnet die normalisierte Normale des Dreiecks
     */
    glm::vec3 GetNormalizedNormal() const
    {
        glm::vec3 normal = GetNormal();
        float len = glm::length(normal);
        if (len > 0.0f)
        {
            return normal / len;
        }
        return glm::vec3(0.0f, 1.0f, 0.0f); // Default bei degeneriertem Dreieck
    }

    /**
     * Berechnet den Schwerpunkt (Centroid) des Dreiecks
     */
    glm::vec3 GetCentroid() const
    {
        return (v0 + v1 + v2) / 3.0f;
    }
};

/**
 * Namespace für Dreieck-Kollisionserkennung
 * 
 * Implementiert den Möller-Trumbore Algorithmus für schnelle
 * Dreieck-gegen-Dreieck Kollisionserkennung.
 */
namespace TriangleCollision
{
    // Epsilon für Fließkomma-Vergleiche
    constexpr float EPSILON = 1e-6f;

    /**
     * Projiziert einen Punkt auf eine Achse
     */
    inline float ProjectOntoAxis(const glm::vec3& point, const glm::vec3& axis)
    {
        return glm::dot(point, axis);
    }

    /**
     * Berechnet das Projektionsintervall eines Dreiecks auf eine Achse
     */
    inline void ProjectTriangle(const Triangle& tri, const glm::vec3& axis, 
                                float& outMin, float& outMax)
    {
        float p0 = ProjectOntoAxis(tri.v0, axis);
        float p1 = ProjectOntoAxis(tri.v1, axis);
        float p2 = ProjectOntoAxis(tri.v2, axis);

        outMin = std::min({p0, p1, p2});
        outMax = std::max({p0, p1, p2});
    }

    /**
     * Prüft ob zwei Intervalle sich überlappen
     */
    inline bool IntervalsOverlap(float min1, float max1, float min2, float max2)
    {
        return !(max1 < min2 || max2 < min1);
    }

    /**
     * Separating Axis Test für eine bestimmte Achse
     * Gibt true zurück, wenn die Dreiecke auf dieser Achse getrennt sind
     */
    inline bool TestSeparatingAxis(const Triangle& tri1, const Triangle& tri2, 
                                   const glm::vec3& axis)
    {
        float axisLen = glm::length(axis);
        if (axisLen < EPSILON)
        {
            // Degenerierte Achse - kein Separator
            return false;
        }

        glm::vec3 normalizedAxis = axis / axisLen;

        float min1, max1, min2, max2;
        ProjectTriangle(tri1, normalizedAxis, min1, max1);
        ProjectTriangle(tri2, normalizedAxis, min2, max2);

        // Wenn die Intervalle sich nicht überlappen, ist dies eine separierende Achse
        return !IntervalsOverlap(min1, max1, min2, max2);
    }

    /**
     * Dreieck-gegen-Dreieck Kollisionserkennung
     * 
     * Verwendet den Separating Axis Theorem (SAT) Ansatz:
     * Zwei konvexe Objekte überschneiden sich nicht, wenn es eine Achse gibt,
     * auf der ihre Projektionen getrennt sind.
     * 
     * Für zwei Dreiecke müssen wir folgende Achsen testen:
     * 1. Die Normalen beider Dreiecke (2 Achsen)
     * 2. Die Kreuzprodukte der Kanten beider Dreiecke (9 Achsen)
     * 
     * @param tri1 Erstes Dreieck
     * @param tri2 Zweites Dreieck
     * @return true wenn die Dreiecke sich überschneiden
     */
    inline bool Intersects(const Triangle& tri1, const Triangle& tri2)
    {
        // Kanten des ersten Dreiecks
        glm::vec3 edge1_0 = tri1.v1 - tri1.v0;
        glm::vec3 edge1_1 = tri1.v2 - tri1.v1;
        glm::vec3 edge1_2 = tri1.v0 - tri1.v2;

        // Kanten des zweiten Dreiecks
        glm::vec3 edge2_0 = tri2.v1 - tri2.v0;
        glm::vec3 edge2_1 = tri2.v2 - tri2.v1;
        glm::vec3 edge2_2 = tri2.v0 - tri2.v2;

        // Normale des ersten Dreiecks
        glm::vec3 normal1 = glm::cross(edge1_0, edge1_1);
        // Normale des zweiten Dreiecks
        glm::vec3 normal2 = glm::cross(edge2_0, edge2_1);

        // Test Achse 1: Normale von Dreieck 1
        if (TestSeparatingAxis(tri1, tri2, normal1)) return false;

        // Test Achse 2: Normale von Dreieck 2
        if (TestSeparatingAxis(tri1, tri2, normal2)) return false;

        // Test die 9 Kreuzprodukt-Achsen (Kante1 x Kante2)
        glm::vec3 edges1[3] = { edge1_0, edge1_1, edge1_2 };
        glm::vec3 edges2[3] = { edge2_0, edge2_1, edge2_2 };

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                glm::vec3 axis = glm::cross(edges1[i], edges2[j]);
                if (TestSeparatingAxis(tri1, tri2, axis)) return false;
            }
        }

        // Keine separierende Achse gefunden -> Kollision
        return true;
    }

    /**
     * Prüft ob ein Dreieck und eine AABB sich überschneiden
     * Verwendet den SAT-Algorithmus
     */
    inline bool IntersectsAABB(const Triangle& tri, const glm::vec3& aabbMin, 
                               const glm::vec3& aabbMax)
    {
        // Verschiebe alles so, dass die AABB am Ursprung zentriert ist
        glm::vec3 center = (aabbMin + aabbMax) * 0.5f;
        glm::vec3 halfExtents = (aabbMax - aabbMin) * 0.5f;

        // Verschiebe das Dreieck
        glm::vec3 v0 = tri.v0 - center;
        glm::vec3 v1 = tri.v1 - center;
        glm::vec3 v2 = tri.v2 - center;

        // Kanten des Dreiecks
        glm::vec3 e0 = v1 - v0;
        glm::vec3 e1 = v2 - v1;
        glm::vec3 e2 = v0 - v2;

        // Test AABB-Achsen (X, Y, Z)
        // X-Achse
        float minX = std::min({v0.x, v1.x, v2.x});
        float maxX = std::max({v0.x, v1.x, v2.x});
        if (minX > halfExtents.x || maxX < -halfExtents.x) return false;

        // Y-Achse
        float minY = std::min({v0.y, v1.y, v2.y});
        float maxY = std::max({v0.y, v1.y, v2.y});
        if (minY > halfExtents.y || maxY < -halfExtents.y) return false;

        // Z-Achse
        float minZ = std::min({v0.z, v1.z, v2.z});
        float maxZ = std::max({v0.z, v1.z, v2.z});
        if (minZ > halfExtents.z || maxZ < -halfExtents.z) return false;

        // Test Dreiecks-Normale
        glm::vec3 normal = glm::cross(e0, e1);
        float d = glm::dot(normal, v0);
        float r = halfExtents.x * std::abs(normal.x) +
                  halfExtents.y * std::abs(normal.y) +
                  halfExtents.z * std::abs(normal.z);
        if (std::abs(d) > r) return false;

        // Test die 9 Kreuzprodukt-Achsen
        // Für jede AABB-Achse (X, Y, Z) und jede Dreieckskante
        auto testAxis = [&](const glm::vec3& axis) -> bool
        {
            float axisLen = glm::length(axis);
            if (axisLen < EPSILON) return true; // Degenerierte Achse

            float p0 = glm::dot(v0, axis);
            float p1 = glm::dot(v1, axis);
            float p2 = glm::dot(v2, axis);
            float triRadius = std::max({p0, p1, p2}) - std::min({p0, p1, p2});
            float triCenter = (std::max({p0, p1, p2}) + std::min({p0, p1, p2})) * 0.5f;

            float boxRadius = halfExtents.x * std::abs(axis.x) +
                              halfExtents.y * std::abs(axis.y) +
                              halfExtents.z * std::abs(axis.z);

            return std::abs(triCenter) <= boxRadius + triRadius * 0.5f;
        };

        // Kreuzprodukte mit X-Achse
        if (!testAxis(glm::cross(glm::vec3(1, 0, 0), e0))) return false;
        if (!testAxis(glm::cross(glm::vec3(1, 0, 0), e1))) return false;
        if (!testAxis(glm::cross(glm::vec3(1, 0, 0), e2))) return false;

        // Kreuzprodukte mit Y-Achse
        if (!testAxis(glm::cross(glm::vec3(0, 1, 0), e0))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 1, 0), e1))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 1, 0), e2))) return false;

        // Kreuzprodukte mit Z-Achse
        if (!testAxis(glm::cross(glm::vec3(0, 0, 1), e0))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 0, 1), e1))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 0, 1), e2))) return false;

        return true;
    }
}

