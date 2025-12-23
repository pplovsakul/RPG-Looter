#pragma once

#include <vector>
#include <memory>
#include <array>
#include <algorithm>
#include "vendor/glm/glm.hpp"
#include "AABB.h"
#include "Triangle.h"

/**
 * Octree - Hierarchical spatial data structure for collision detection
 * 
 * An octree recursively divides 3D space into 8 equal-sized children.
 * This enables efficient broad-phase collision detection by quickly
 * excluding regions without possible collisions.
 * 
 * Properties:
 * - Each node has an AABB defining its region
 * - Leaf nodes contain triangle indices
 * - Internal nodes have 8 child nodes
 * - Maximum depth and minimum triangles per node are configurable
 * 
 * Child indices (based on position relative to center):
 * Bit 0: X-axis, Bit 1: Y-axis, Bit 2: Z-axis (0=negative, 1=positive)
 * 0: -X, -Y, -Z    1: +X, -Y, -Z    2: -X, +Y, -Z    3: +X, +Y, -Z
 * 4: -X, -Y, +Z    5: +X, -Y, +Z    6: -X, +Y, +Z    7: +X, +Y, +Z
 */
class OctreeNode
{
public:
    static constexpr int MAX_DEPTH = 8;           // Maximum recursion depth
    static constexpr int MIN_TRIANGLES = 4;       // Minimum triangles to split
    static constexpr int MAX_TRIANGLES_LEAF = 16; // Max triangles in a leaf node

    OctreeNode(const AABB& bounds, int depth = 0);
    ~OctreeNode() = default;

    // Inserts a triangle index and subdivides if necessary
    void Insert(size_t triangleIndex, const Triangle& triangle);

    // Returns all triangle indices that could collide with the AABB
    void Query(const AABB& queryBounds, std::vector<size_t>& outIndices) const;

    // Returns all triangle indices in this node and its children
    void GetAllTriangles(std::vector<size_t>& outIndices) const;

    // Getters
    const AABB& GetBounds() const { return m_bounds; }
    bool IsLeaf() const { return m_children[0] == nullptr; }
    int GetDepth() const { return m_depth; }
    size_t GetTriangleCount() const { return m_triangleIndices.size(); }

private:
    // Splits this node into 8 child nodes
    void Subdivide();

    // Determines which child indices a triangle could overlap
    std::vector<int> GetChildIndicesForTriangle(const Triangle& triangle) const;

    // Calculates the AABB for a specific child node
    AABB GetChildBounds(int childIndex) const;

    AABB m_bounds;                                      // Bounds of this node
    int m_depth;                                        // Depth in the tree (0 = root)
    std::vector<size_t> m_triangleIndices;              // Triangle indices in this node
    std::array<std::unique_ptr<OctreeNode>, 8> m_children; // 8 child nodes
    std::vector<Triangle> m_tempTriangles;              // Temporary triangles for subdivision
};

/**
 * Octree - Main class for the octree data structure
 * 
 * This class manages the root node and provides a simple
 * interface for building and querying the octree.
 */
class Octree
{
public:
    Octree() = default;
    ~Octree() = default;

    /**
     * Builds the octree from mesh data
     * 
     * @param vertices Interleaved vertex data (x, y, z, u, v, r, g, b)
     * @param indices  Triangle indices
     * @param stride   Number of floats per vertex (default: 8)
     */
    void Build(const std::vector<float>& vertices,
               const std::vector<unsigned int>& indices,
               size_t stride = 8);

    /**
     * Returns all triangle indices that could collide with the AABB
     */
    void Query(const AABB& queryBounds, std::vector<size_t>& outTriangleIndices) const;

    /**
     * Returns the triangles
     */
    const std::vector<Triangle>& GetTriangles() const { return m_triangles; }

    /**
     * Returns the AABB of the entire mesh
     */
    const AABB& GetBounds() const { return m_bounds; }

    /**
     * Checks if the octree has been built
     */
    bool IsBuilt() const { return m_root != nullptr; }

    /**
     * Clears the octree
     */
    void Clear();

private:
    std::unique_ptr<OctreeNode> m_root;   // Root node
    std::vector<Triangle> m_triangles;     // All triangles of the mesh
    AABB m_bounds;                         // Total AABB of the mesh
};

