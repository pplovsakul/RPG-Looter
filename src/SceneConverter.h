#pragma once
#include "vendor/glm/glm.hpp"
#include "Triangle.h"
#include "Box.h"
#include "Sphere.h"
#include "Material.h"
#include <vector>
#include <iostream>

/**
 * ============================================================================
 * SCENE CONVERTER - PHASE 4
 * ============================================================================
 * 
 * SceneConverter provides utilities to convert primitive-based scenes
 * (Boxes, Spheres) into unified triangle mesh representations suitable
 * for GPU ray tracing with BVH acceleration.
 * 
 * DESIGN GOALS:
 * - Single source of truth for scene geometry (triangle-based)
 * - Stable material index mapping from primitives to meshes
 * - Configurable mesh quality (subdivision levels)
 * - Clean separation of concerns (conversion logic isolated)
 * 
 * ARCHITECTURE:
 * - Static utility class (no state)
 * - Pure functions for primitive conversion
 * - Preserves material indices during conversion
 * - Generates debug/validation output
 * 
 * USAGE:
 * ```cpp
 * std::vector<Triangle> triangles;
 * SceneConverter::convertBoxToTriangles(box, materialIndex, triangles);
 * SceneConverter::convertSphereToTriangles(sphere, materialIndex, triangles, 2);
 * ```
 * ============================================================================
 */
namespace SceneConverter {

    /**
     * convertBoxToTriangles
     * ----------------------
     * Converts a Box primitive to 12 triangles (2 per face).
     * 
     * @param box - Source Box primitive with bounds
     * @param materialIndex - Material index to assign to all triangles
     * @param outTriangles - Output vector to append triangles to
     * 
     * Architecture Notes:
     * - Uses MeshGenerator::createBox for consistent topology
     * - Preserves box's world-space position and size
     * - Material index is explicitly provided (not from box.material)
     *   to support global material remapping
     */
    inline void convertBoxToTriangles(
        const Box& box,
        int materialIndex,
        std::vector<Triangle>& outTriangles
    ) {
        // Calculate box center and size from bounds
        glm::vec3 center = (box.minBounds + box.maxBounds) * 0.5f;
        glm::vec3 size = box.maxBounds - box.minBounds;
        
        // Generate triangle mesh using MeshGenerator
        auto boxTriangles = MeshGenerator::createBox(center, size, materialIndex);
        
        // Append to output
        outTriangles.insert(outTriangles.end(), boxTriangles.begin(), boxTriangles.end());
    }

    /**
     * convertSphereToTriangles
     * -------------------------
     * Converts a Sphere primitive to triangulated mesh using Icosphere subdivision.
     * 
     * @param sphere - Source Sphere primitive with center and radius
     * @param materialIndex - Material index to assign to all triangles
     * @param outTriangles - Output vector to append triangles to
     * @param subdivisionLevel - Quality of sphere approximation
     *                          0 = 20 triangles (icosahedron)
     *                          1 = 80 triangles
     *                          2 = 320 triangles (default, good quality)
     *                          3 = 1280 triangles (high quality)
     * 
     * Performance Considerations:
     * - Subdivision level 2 is a good balance for most scenes
     * - Higher levels provide smoother silhouettes but increase triangle count exponentially
     * - For distant/small spheres, level 1 may be sufficient
     * 
     * Quality Guidelines:
     * - Level 0: Only for debugging or very distant objects
     * - Level 1: Background/small objects (80 tris)
     * - Level 2: Standard quality for most spheres (320 tris) - RECOMMENDED
     * - Level 3: Hero objects or close-ups (1280 tris)
     */
    inline void convertSphereToTriangles(
        const Sphere& sphere,
        int materialIndex,
        std::vector<Triangle>& outTriangles,
        int subdivisionLevel = 2
    ) {
        // Generate icosphere mesh using MeshGenerator
        auto sphereTriangles = MeshGenerator::createIcosphere(
            sphere.center,
            sphere.radius,
            subdivisionLevel,
            materialIndex
        );
        
        // Append to output
        outTriangles.insert(outTriangles.end(), sphereTriangles.begin(), sphereTriangles.end());
    }

    /**
     * ConversionStatistics
     * ---------------------
     * Statistics structure for scene conversion reporting.
     * Used for validation and debugging output.
     */
    struct ConversionStatistics {
        size_t totalBoxes = 0;
        size_t totalSpheres = 0;
        size_t totalTriangles = 0;
        size_t boxTriangles = 0;
        size_t sphereTriangles = 0;
        
        void print() const {
            std::cout << "\n=== Scene Conversion Statistics ===" << std::endl;
            std::cout << "Input Primitives:" << std::endl;
            std::cout << "  Boxes:   " << totalBoxes << std::endl;
            std::cout << "  Spheres: " << totalSpheres << std::endl;
            std::cout << "\nOutput Triangles:" << std::endl;
            std::cout << "  From Boxes:   " << boxTriangles 
                      << " (" << (totalBoxes > 0 ? boxTriangles / totalBoxes : 0) 
                      << " tris/box)" << std::endl;
            std::cout << "  From Spheres: " << sphereTriangles 
                      << " (" << (totalSpheres > 0 ? sphereTriangles / totalSpheres : 0) 
                      << " tris/sphere)" << std::endl;
            std::cout << "  Total:        " << totalTriangles << std::endl;
            std::cout << "==================================\n" << std::endl;
        }
    };

    /**
     * convertSceneToTriangles
     * ------------------------
     * Converts entire scene (all boxes and spheres) to unified triangle mesh.
     * 
     * @param boxes - Vector of Box primitives
     * @param spheres - Vector of Sphere primitives
     * @param materialMapping - Function to map primitive index to material index
     *                         Signature: int(size_t primitiveIndex, bool isSphere)
     * @param sphereSubdivision - Subdivision level for sphere approximation (default: 2)
     * @param outStats - Optional output statistics for debugging
     * 
     * @return Vector of triangles representing the entire scene
     * 
     * Material Mapping Strategy:
     * - Each primitive (box or sphere) needs a material index
     * - The materialMapping function provides flexibility for complex scenes
     * - Example: Lambda that looks up material from primitive's material field
     * - Example: Simple index-based mapping for procedural scenes
     * 
     * Architecture Notes:
     * - Single allocation strategy: reserve space upfront for efficiency
     * - Boxes converted first, then spheres (for consistent ordering)
     * - Material indices must be valid (0 <= index < materialCount)
     * - Statistics collection is optional but recommended for debugging
     */
    inline std::vector<Triangle> convertSceneToTriangles(
        const std::vector<Box>& boxes,
        const std::vector<Sphere>& spheres,
        std::function<int(size_t, bool)> materialMapping,
        int sphereSubdivision = 2,
        ConversionStatistics* outStats = nullptr
    ) {
        std::vector<Triangle> triangles;
        
        // Estimate triangle count and reserve space
        // Boxes: 12 triangles each
        // Spheres: varies by subdivision (20 * 4^level)
        size_t estimatedTriCount = boxes.size() * 12;
        for (size_t i = 0; i < spheres.size(); ++i) {
            estimatedTriCount += 20 * (1 << (2 * sphereSubdivision)); // 4^level
        }
        triangles.reserve(estimatedTriCount);
        
        ConversionStatistics stats;
        stats.totalBoxes = boxes.size();
        stats.totalSpheres = spheres.size();
        
        // Convert all boxes
        size_t boxTriStartCount = triangles.size();
        for (size_t i = 0; i < boxes.size(); ++i) {
            int matIdx = materialMapping(i, false); // false = box
            convertBoxToTriangles(boxes[i], matIdx, triangles);
        }
        stats.boxTriangles = triangles.size() - boxTriStartCount;
        
        // Convert all spheres
        size_t sphereTriStartCount = triangles.size();
        for (size_t i = 0; i < spheres.size(); ++i) {
            int matIdx = materialMapping(i, true); // true = sphere
            convertSphereToTriangles(spheres[i], matIdx, triangles, sphereSubdivision);
        }
        stats.sphereTriangles = triangles.size() - sphereTriStartCount;
        
        stats.totalTriangles = triangles.size();
        
        // Output statistics if requested
        if (outStats) {
            *outStats = stats;
        }
        
        return triangles;
    }

} // namespace SceneConverter

/**
 * ============================================================================
 * INTEGRATION NOTES FOR PHASE 4
 * ============================================================================
 * 
 * Scene Conversion Workflow:
 * ---------------------------
 * 1. Call convertSceneToTriangles with current scene primitives
 * 2. System generates unified triangle mesh with proper material indices
 * 3. BVHBuilder creates acceleration structure from triangles
 * 4. Upload to GPU via existing GPURayTracer::loadTriangleMesh()
 * 5. Render using Phase 3 shader pipeline (no changes needed)
 * 
 * Material Index Mapping:
 * -----------------------
 * Option A: Direct mapping from primitive material
 * ```cpp
 * auto matMapping = [&](size_t idx, bool isSphere) {
 *     if (isSphere) return findMaterialIndex(spheres[idx].material);
 *     else return findMaterialIndex(boxes[idx].material);
 * };
 * ```
 * 
 * Option B: Index-based mapping (current implementation)
 * ```cpp
 * auto matMapping = [&](size_t idx, bool isSphere) {
 *     return getMaterialIndexForObject(idx);
 * };
 * ```
 * 
 * Hot-Reload Strategy:
 * --------------------
 * Add hotkey (e.g., 'C' for Convert) that calls:
 * 1. convertSceneToTriangles(boxes, spheres, ...)
 * 2. gpuRT->loadTriangleMesh(triangles)
 * 3. Clear primitive arrays (boxes.clear(), spheres.clear())
 * 4. Render with triangle-only pipeline
 * 
 * This enables A/B testing: primitive-based vs. triangle-based rendering.
 * 
 * Performance Considerations:
 * ---------------------------
 * - Conversion is CPU-bound (one-time cost at scene load)
 * - Typical scene (50 primitives) converts in < 1ms
 * - Complex scenes (1000+ primitives) may take 10-50ms
 * - Consider async conversion for very large scenes
 * - BVH build is typically 2-5x slower than conversion
 * 
 * Memory Management:
 * ------------------
 * - Triangle mesh is persistent (stored in GPURayTracer)
 * - Original primitives can be cleared after conversion
 * - GPU buffers are automatically managed by GPURayTracer
 * - Consider using reserve() for large scenes to avoid reallocations
 * 
 * Quality vs. Performance Tradeoff:
 * ---------------------------------
 * Sphere Subdivision Levels:
 * - Level 0: 20 triangles   (very low quality, debugging only)
 * - Level 1: 80 triangles   (acceptable for small/distant spheres)
 * - Level 2: 320 triangles  (RECOMMENDED - good quality/performance balance)
 * - Level 3: 1280 triangles (high quality, use sparingly)
 * 
 * For a scene with 10 spheres:
 * - Level 1: 800 triangles
 * - Level 2: 3,200 triangles (recommended)
 * - Level 3: 12,800 triangles
 * 
 * Validation:
 * -----------
 * Always check ConversionStatistics output:
 * - Verify triangle counts match expectations
 * - Ensure no material index out of bounds
 * - Check for degenerate triangles (should be none)
 * - Validate BVH build success
 * ============================================================================
 */
