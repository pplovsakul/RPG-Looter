#pragma once

#include "vendor/glm/glm.hpp"
#include <cmath>

/**
 * Triangle - Triangle structure for collision detection
 * 
 * Stores the three vertices of a triangle and provides
 * helper functions for geometric calculations.
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
     * Calculates the normal of the triangle (not normalized)
     */
    glm::vec3 GetNormal() const
    {
        return glm::cross(v1 - v0, v2 - v0);
    }

    /**
     * Calculates the normalized normal of the triangle
     */
    glm::vec3 GetNormalizedNormal() const
    {
        glm::vec3 normal = GetNormal();
        float len = glm::length(normal);
        if (len > 0.0f)
        {
            return normal / len;
        }
        return glm::vec3(0.0f, 1.0f, 0.0f); // Default for degenerate triangle
    }

    /**
     * Calculates the centroid of the triangle
     */
    glm::vec3 GetCentroid() const
    {
        return (v0 + v1 + v2) / 3.0f;
    }
};

/**
 * Namespace for triangle collision detection
 * 
 * Implements the Separating Axis Theorem (SAT) for fast
 * triangle-to-triangle collision detection.
 */
namespace TriangleCollision
{
    // Epsilon for floating-point comparisons
    constexpr float EPSILON = 1e-6f;

    /**
     * Projects a point onto an axis
     */
    inline float ProjectOntoAxis(const glm::vec3& point, const glm::vec3& axis)
    {
        return glm::dot(point, axis);
    }

    /**
     * Calculates the projection interval of a triangle onto an axis
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
     * Checks if two intervals overlap
     */
    inline bool IntervalsOverlap(float min1, float max1, float min2, float max2)
    {
        return !(max1 < min2 || max2 < min1);
    }

    /**
     * Separating Axis Test for a specific axis
     * Returns true if the triangles are separated on this axis
     */
    inline bool TestSeparatingAxis(const Triangle& tri1, const Triangle& tri2, 
                                   const glm::vec3& axis)
    {
        float axisLen = glm::length(axis);
        if (axisLen < EPSILON)
        {
            // Degenerate axis - no separator
            return false;
        }

        glm::vec3 normalizedAxis = axis / axisLen;

        float min1, max1, min2, max2;
        ProjectTriangle(tri1, normalizedAxis, min1, max1);
        ProjectTriangle(tri2, normalizedAxis, min2, max2);

        // If intervals don't overlap, this is a separating axis
        return !IntervalsOverlap(min1, max1, min2, max2);
    }

    /**
     * Triangle-to-triangle collision detection
     * 
     * Uses the Separating Axis Theorem (SAT) approach:
     * Two convex objects do not intersect if there is an axis on which
     * their projections are separated.
     * 
     * For two triangles we must test the following axes:
     * 1. The normals of both triangles (2 axes)
     * 2. The cross products of the edges of both triangles (9 axes)
     * 
     * @param tri1 First triangle
     * @param tri2 Second triangle
     * @return true if the triangles intersect
     */
    inline bool Intersects(const Triangle& tri1, const Triangle& tri2)
    {
        // Edges of the first triangle
        glm::vec3 edge1_0 = tri1.v1 - tri1.v0;
        glm::vec3 edge1_1 = tri1.v2 - tri1.v1;
        glm::vec3 edge1_2 = tri1.v0 - tri1.v2;

        // Edges of the second triangle
        glm::vec3 edge2_0 = tri2.v1 - tri2.v0;
        glm::vec3 edge2_1 = tri2.v2 - tri2.v1;
        glm::vec3 edge2_2 = tri2.v0 - tri2.v2;

        // Normal of the first triangle (using v1-v0 and v2-v0)
        glm::vec3 normal1 = glm::cross(edge1_0, tri1.v2 - tri1.v0);
        // Normal of the second triangle (using v1-v0 and v2-v0)
        glm::vec3 normal2 = glm::cross(edge2_0, tri2.v2 - tri2.v0);

        // Test axis 1: Normal of triangle 1
        if (TestSeparatingAxis(tri1, tri2, normal1)) return false;

        // Test axis 2: Normal of triangle 2
        if (TestSeparatingAxis(tri1, tri2, normal2)) return false;

        // Test the 9 cross product axes (edge1 x edge2)
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

        // No separating axis found -> collision
        return true;
    }

    /**
     * Checks if a triangle and an AABB intersect
     * Uses the SAT algorithm
     */
    inline bool IntersectsAABB(const Triangle& tri, const glm::vec3& aabbMin, 
                               const glm::vec3& aabbMax)
    {
        // Translate everything so the AABB is centered at origin
        glm::vec3 center = (aabbMin + aabbMax) * 0.5f;
        glm::vec3 halfExtents = (aabbMax - aabbMin) * 0.5f;

        // Translate the triangle
        glm::vec3 v0 = tri.v0 - center;
        glm::vec3 v1 = tri.v1 - center;
        glm::vec3 v2 = tri.v2 - center;

        // Edges of the triangle
        glm::vec3 e0 = v1 - v0;
        glm::vec3 e1 = v2 - v1;
        glm::vec3 e2 = v0 - v2;

        // Test AABB axes (X, Y, Z)
        // X-axis
        float minX = std::min({v0.x, v1.x, v2.x});
        float maxX = std::max({v0.x, v1.x, v2.x});
        if (minX > halfExtents.x || maxX < -halfExtents.x) return false;

        // Y-axis
        float minY = std::min({v0.y, v1.y, v2.y});
        float maxY = std::max({v0.y, v1.y, v2.y});
        if (minY > halfExtents.y || maxY < -halfExtents.y) return false;

        // Z-axis
        float minZ = std::min({v0.z, v1.z, v2.z});
        float maxZ = std::max({v0.z, v1.z, v2.z});
        if (minZ > halfExtents.z || maxZ < -halfExtents.z) return false;

        // Test triangle normal
        glm::vec3 normal = glm::cross(e0, e1);
        float d = glm::dot(normal, v0);
        float r = halfExtents.x * std::abs(normal.x) +
                  halfExtents.y * std::abs(normal.y) +
                  halfExtents.z * std::abs(normal.z);
        if (std::abs(d) > r) return false;

        // Test the 9 cross product axes
        // For each AABB axis (X, Y, Z) and each triangle edge
        auto testAxis = [&](const glm::vec3& axis) -> bool
        {
            float axisLen = glm::length(axis);
            if (axisLen < EPSILON) return true; // Degenerate axis

            float p0 = glm::dot(v0, axis);
            float p1 = glm::dot(v1, axis);
            float p2 = glm::dot(v2, axis);
            float triMin = std::min({p0, p1, p2});
            float triMax = std::max({p0, p1, p2});
            float triRadius = triMax - triMin;
            float triCenter = (triMax + triMin) * 0.5f;

            float boxRadius = halfExtents.x * std::abs(axis.x) +
                              halfExtents.y * std::abs(axis.y) +
                              halfExtents.z * std::abs(axis.z);

            return std::abs(triCenter) <= boxRadius + triRadius * 0.5f;
        };

        // Cross products with X-axis
        if (!testAxis(glm::cross(glm::vec3(1, 0, 0), e0))) return false;
        if (!testAxis(glm::cross(glm::vec3(1, 0, 0), e1))) return false;
        if (!testAxis(glm::cross(glm::vec3(1, 0, 0), e2))) return false;

        // Cross products with Y-axis
        if (!testAxis(glm::cross(glm::vec3(0, 1, 0), e0))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 1, 0), e1))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 1, 0), e2))) return false;

        // Cross products with Z-axis
        if (!testAxis(glm::cross(glm::vec3(0, 0, 1), e0))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 0, 1), e1))) return false;
        if (!testAxis(glm::cross(glm::vec3(0, 0, 1), e2))) return false;

        return true;
    }
}

