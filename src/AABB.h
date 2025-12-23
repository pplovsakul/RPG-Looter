#pragma once

#include <vector>
#include <limits>
#include "vendor/glm/glm.hpp"

/**
 * AABB (Axis-Aligned Bounding Box)
 * 
 * The AABB is defined by two points:
 * - min: Point with the smallest coordinates in all three axes
 * - max: Point with the largest coordinates in all three axes
 * 
 * This simple structure enables very fast intersection tests
 * and is ideal as the first stage of collision detection (Broad Phase).
 */
struct AABB
{
    glm::vec3 min;  // Smallest coordinates (-X, -Y, -Z corner)
    glm::vec3 max;  // Largest coordinates (+X, +Y, +Z corner)

    // Default constructor: Initializes an "inverted" AABB that
    // automatically grows correctly when points are added
    AABB()
        : min(std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max())
        , max(std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest())
    {
    }

    // Constructor with explicit bounds
    AABB(const glm::vec3& minCorner, const glm::vec3& maxCorner)
        : min(minCorner)
        , max(maxCorner)
    {
    }

    /**
     * Checks if this AABB has valid bounds
     * An AABB is invalid if min > max in any axis
     */
    bool IsValid() const
    {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    /**
     * Calculates the center of the AABB
     */
    glm::vec3 GetCenter() const
    {
        return (min + max) * 0.5f;
    }

    /**
     * Calculates the extents (size) of the AABB
     */
    glm::vec3 GetExtents() const
    {
        return max - min;
    }

    /**
     * Calculates the half-extents of the AABB
     */
    glm::vec3 GetHalfExtents() const
    {
        return (max - min) * 0.5f;
    }

    /**
     * Expands the AABB to include a single point
     */
    void ExpandToInclude(const glm::vec3& point)
    {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    /**
     * Expands the AABB to include another AABB
     */
    void ExpandToInclude(const AABB& other)
    {
        ExpandToInclude(other.min);
        ExpandToInclude(other.max);
    }

    /**
     * Transforms the AABB with a position (translation)
     * Returns a new, transformed AABB
     */
    AABB Transformed(const glm::vec3& position) const
    {
        return AABB(min + position, max + position);
    }
};

/**
 * Namespace for AABB utility functions
 */
namespace AABBUtils
{
    /**
     * Computes the AABB for a mesh from interleaved vertex data
     * 
     * @param vertices Interleaved vertex data (x, y, z, u, v, r, g, b per vertex)
     * @param stride   Number of floats per vertex (default: 8 for Position + TexCoord + Color)
     * @return         The computed AABB
     * 
     * The format corresponds to the layout used in Mesh.cpp:
     * - Offset 0: Position (vec3: x, y, z)
     * - Offset 3: Texture Coordinates (vec2: u, v)
     * - Offset 5: Color (vec3: r, g, b)
     */
    inline AABB ComputeFromVertices(const std::vector<float>& vertices, size_t stride = 8)
    {
        AABB result;
        
        if (vertices.empty() || stride < 3)
        {
            return result; // Return invalid AABB
        }

        const size_t vertexCount = vertices.size() / stride;
        
        for (size_t i = 0; i < vertexCount; ++i)
        {
            const size_t baseIndex = i * stride;
            glm::vec3 position(
                vertices[baseIndex],
                vertices[baseIndex + 1],
                vertices[baseIndex + 2]
            );
            result.ExpandToInclude(position);
        }

        return result;
    }

    /**
     * Computes the AABB for a single triangle
     * 
     * @param v0, v1, v2 The three vertices of the triangle
     * @return           The AABB that encloses the triangle
     */
    inline AABB ComputeFromTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
    {
        AABB result;
        result.ExpandToInclude(v0);
        result.ExpandToInclude(v1);
        result.ExpandToInclude(v2);
        return result;
    }

    /**
     * AABB-AABB collision detection (intersection test)
     * 
     * Two AABBs intersect if they overlap in all three axes.
     * This is a very fast test (only 6 comparisons).
     * 
     * @param a First AABB
     * @param b Second AABB
     * @return  true if the AABBs intersect, false otherwise
     */
    inline bool Intersects(const AABB& a, const AABB& b)
    {
        // Separating Axis Test: If there is an axis where the
        // projections do not overlap, then there is no collision.
        // 
        // Overlap on an axis: a.min <= b.max && b.min <= a.max
        
        if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
        if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
        if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
        
        return true;
    }

    /**
     * Checks if a point is inside an AABB
     * 
     * @param aabb The AABB
     * @param point The point to check
     * @return true if the point is inside the AABB
     */
    inline bool ContainsPoint(const AABB& aabb, const glm::vec3& point)
    {
        return point.x >= aabb.min.x && point.x <= aabb.max.x &&
               point.y >= aabb.min.y && point.y <= aabb.max.y &&
               point.z >= aabb.min.z && point.z <= aabb.max.z;
    }

    /**
     * Checks if an AABB is completely contained within another
     * 
     * @param outer The outer AABB
     * @param inner The inner AABB
     * @return true if inner is completely contained in outer
     */
    inline bool Contains(const AABB& outer, const AABB& inner)
    {
        return inner.min.x >= outer.min.x && inner.max.x <= outer.max.x &&
               inner.min.y >= outer.min.y && inner.max.y <= outer.max.y &&
               inner.min.z >= outer.min.z && inner.max.z <= outer.max.z;
    }
}

