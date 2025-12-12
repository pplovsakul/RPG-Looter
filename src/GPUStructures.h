#pragma once
#include "vendor/glm/glm.hpp"
#include <cstdint>
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <cfloat>

/**
 * ============================================================================
 * GPU-COMPATIBLE DATA STRUCTURES FOR RAY TRACING
 * ============================================================================
 * 
 * This header defines POD (Plain Old Data) structures that are binary-compatible
 * with GLSL shader layouts using std430 packing rules (OpenGL 4.3+).
 * 
 * CRITICAL ALIGNMENT RULES (std430):
 * ----------------------------------
 * - Scalars (float, int, uint): 4-byte alignment
 * - vec2: 8-byte alignment
 * - vec3: 16-byte alignment (!)  <-- Same as vec4 due to GLSL padding
 * - vec4: 16-byte alignment
 * - Structs: Aligned to largest member, padded to multiple of base alignment
 * 
 * DESIGN PRINCIPLES:
 * ------------------
 * 1. All structures are POD types (trivially copyable)
 * 2. Explicit padding fields to match GLSL layout
 * 3. static_assert validations for size and alignment
 * 4. No virtual functions, constructors with side effects, or dynamic allocation
 * 5. Compatible with both CPU and GPU memory spaces
 * 
 * USAGE PATTERN:
 * --------------
 * 1. Host-side: Fill structures with scene data
 * 2. Serialize to contiguous arrays (std::vector<StructGPU>)
 * 3. Upload to GPU via SSBO (glBufferData)
 * 4. Access in GLSL compute shaders via buffer blocks
 * 
 * VALIDATION:
 * -----------
 * All structures include compile-time size/alignment checks.
 * If these fail, the GLSL layout must be adjusted or vice versa.
 * 
 * Author Notes:
 * This infrastructure follows modern game engine patterns (Unreal, Unity, Frostbite)
 * for CPU-GPU data interchange. Critical for efficient ray tracing pipelines.
 * ============================================================================
 */

namespace GPUStructs {

    /**
     * TriangleGPU: GPU-compatible triangle primitive
     * ------------------------------------------------
     * Represents a single triangle with vertices, normal, and material reference.
     * 
     * Memory Layout (std430):
     * - v0: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 0
     * - v1: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 16
     * - v2: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 32
     * - normal: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 48
     * - materialIndex: int (4 bytes) @ offset 64
     * - padding: 12 bytes to align to 16-byte boundary
     * Total: 80 bytes
     * 
     * GLSL Correspondence:
     * struct TriangleGPU {
     *     vec3 v0;              // offset 0
     *     vec3 v1;              // offset 16
     *     vec3 v2;              // offset 32
     *     vec3 normal;          // offset 48
     *     int materialIndex;    // offset 64
     * };
     * 
     * NOTE: GLSL automatically pads vec3 to 16 bytes in buffer layouts!
     */
    struct alignas(16) TriangleGPU {
        glm::vec3 v0;           // Vertex 0
        float _pad0;            // Padding to 16 bytes
        glm::vec3 v1;           // Vertex 1
        float _pad1;            // Padding to 16 bytes
        glm::vec3 v2;           // Vertex 2
        float _pad2;            // Padding to 16 bytes
        glm::vec3 normal;       // Pre-computed surface normal
        float _pad3;            // Padding to 16 bytes
        int32_t materialIndex;  // Index into material buffer
        float _pad4;            // Padding for next vec3 alignment
        float _pad5;
        float _pad6;

        // Default constructor for POD compliance
        TriangleGPU() 
            : v0(0.0f), _pad0(0.0f)
            , v1(0.0f), _pad1(0.0f)
            , v2(0.0f), _pad2(0.0f)
            , normal(0.0f, 1.0f, 0.0f), _pad3(0.0f)
            , materialIndex(0), _pad4(0.0f), _pad5(0.0f), _pad6(0.0f)
        {}

        // Conversion constructor from host Triangle
        TriangleGPU(const glm::vec3& vertex0, const glm::vec3& vertex1, 
                    const glm::vec3& vertex2, const glm::vec3& n, int32_t matIdx)
            : v0(vertex0), _pad0(0.0f)
            , v1(vertex1), _pad1(0.0f)
            , v2(vertex2), _pad2(0.0f)
            , normal(n), _pad3(0.0f)
            , materialIndex(matIdx), _pad4(0.0f), _pad5(0.0f), _pad6(0.0f)
        {}
    };

    // Compile-time validation: Ensure structure size matches expected layout
    static_assert(sizeof(TriangleGPU) == 80, 
        "TriangleGPU size mismatch! Expected 80 bytes for std430 layout.");
    static_assert(alignof(TriangleGPU) == 16, 
        "TriangleGPU alignment mismatch! Expected 16-byte alignment.");
    static_assert(std::is_standard_layout<TriangleGPU>::value, 
        "TriangleGPU must be standard layout for GPU compatibility!");
    static_assert(std::is_trivially_copyable<TriangleGPU>::value, 
        "TriangleGPU must be trivially copyable for memcpy safety!");

    /**
     * BVHNodeGPU: GPU-compatible BVH (Bounding Volume Hierarchy) node
     * ----------------------------------------------------------------
     * Compact representation of BVH tree nodes for efficient ray traversal.
     * 
     * Memory Layout (std430):
     * - aabbMin: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 0
     * - aabbMax: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 16
     * - leftChild: int (4 bytes) @ offset 32
     * - rightChild: int (4 bytes) @ offset 36
     * - triangleIndex: int (4 bytes) @ offset 40
     * - triangleCount: int (4 bytes) @ offset 44
     * Total: 48 bytes
     * 
     * GLSL Correspondence:
     * struct BVHNodeGPU {
     *     vec3 aabbMin;         // offset 0  - Minimum bounds of AABB
     *     vec3 aabbMax;         // offset 16 - Maximum bounds of AABB
     *     int leftChild;        // offset 32 - Index to left child (-1 if leaf)
     *     int rightChild;       // offset 36 - Index to right child (-1 if leaf)
     *     int triangleIndex;    // offset 40 - First triangle index (if leaf)
     *     int triangleCount;    // offset 44 - Number of triangles (0 if internal)
     * };
     * 
     * BVH Traversal Logic:
     * - Internal Node: triangleCount == 0, leftChild >= 0, rightChild >= 0
     * - Leaf Node: triangleCount > 0, contains triangles[triangleIndex..triangleIndex+count-1]
     * - Empty Node: triangleCount == 0, leftChild == -1, rightChild == -1
     * 
     * Optimization Notes:
     * - AABB bounds stored as min/max for efficient ray-box intersection
     * - Implicit parent-child relationship (array layout) for cache efficiency
     * - Compact 48-byte node for good memory locality (3 nodes per cache line)
     */
    struct alignas(16) BVHNodeGPU {
        glm::vec3 aabbMin;      // AABB minimum bounds
        float _pad0;            // Padding to 16 bytes
        glm::vec3 aabbMax;      // AABB maximum bounds
        float _pad1;            // Padding to 16 bytes
        int32_t leftChild;      // Index to left child node (-1 for leaf)
        int32_t rightChild;     // Index to right child node (-1 for leaf)
        int32_t triangleIndex;  // First triangle index (if leaf node)
        int32_t triangleCount;  // Number of triangles in leaf (0 for internal nodes)

        // Default constructor
        BVHNodeGPU()
            : aabbMin(FLT_MAX), _pad0(0.0f)
            , aabbMax(-FLT_MAX), _pad1(0.0f)
            , leftChild(-1), rightChild(-1)
            , triangleIndex(0), triangleCount(0)
        {}

        // Constructor with parameters
        BVHNodeGPU(const glm::vec3& minBounds, const glm::vec3& maxBounds,
                   int32_t left, int32_t right, int32_t triIdx, int32_t triCount)
            : aabbMin(minBounds), _pad0(0.0f)
            , aabbMax(maxBounds), _pad1(0.0f)
            , leftChild(left), rightChild(right)
            , triangleIndex(triIdx), triangleCount(triCount)
        {}

        // Utility: Check if this is a leaf node
        bool isLeaf() const { 
            return triangleCount > 0; 
        }

        // Utility: Check if this is an internal node
        bool isInternal() const { 
            return triangleCount == 0 && (leftChild >= 0 || rightChild >= 0); 
        }
    };

    // Compile-time validation for BVHNodeGPU
    static_assert(sizeof(BVHNodeGPU) == 48, 
        "BVHNodeGPU size mismatch! Expected 48 bytes for std430 layout.");
    static_assert(alignof(BVHNodeGPU) == 16, 
        "BVHNodeGPU alignment mismatch! Expected 16-byte alignment.");
    static_assert(std::is_standard_layout<BVHNodeGPU>::value, 
        "BVHNodeGPU must be standard layout for GPU compatibility!");
    static_assert(std::is_trivially_copyable<BVHNodeGPU>::value, 
        "BVHNodeGPU must be trivially copyable for memcpy safety!");

    /**
     * MaterialGPU: GPU-compatible PBR material
     * -----------------------------------------
     * Already defined in GPURayTracer.h as GPUMaterial.
     * Included here for reference and potential future unification.
     * 
     * Memory Layout (std430):
     * - albedo: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 0
     * - roughness: float (4 bytes) @ offset 16
     * - emission: vec3 (12 bytes) + 4 bytes padding = 16 bytes @ offset 20 (rounded to 32)
     * - metallic: float (4 bytes) @ offset 32
     * Total: ~48 bytes (depends on compiler alignment)
     * 
     * NOTE: The existing GPUMaterial in GPURayTracer.h should be verified
     * against this layout. Consider refactoring to use this centralized definition.
     */

    /**
     * Buffer Layout Debugging Utilities
     * ----------------------------------
     * Use these functions to print and validate buffer layouts at runtime.
     * Critical for debugging GPU-CPU data mismatches.
     */
    namespace Debug {
        inline void printTriangleGPULayout() {
            std::cout << "\n=== TriangleGPU Memory Layout ===" << std::endl;
            std::cout << "Size: " << sizeof(TriangleGPU) << " bytes" << std::endl;
            std::cout << "Alignment: " << alignof(TriangleGPU) << " bytes" << std::endl;
            std::cout << "Offsets:" << std::endl;
            std::cout << "  v0:            " << offsetof(TriangleGPU, v0) << std::endl;
            std::cout << "  v1:            " << offsetof(TriangleGPU, v1) << std::endl;
            std::cout << "  v2:            " << offsetof(TriangleGPU, v2) << std::endl;
            std::cout << "  normal:        " << offsetof(TriangleGPU, normal) << std::endl;
            std::cout << "  materialIndex: " << offsetof(TriangleGPU, materialIndex) << std::endl;
            std::cout << "================================\n" << std::endl;
        }

        inline void printBVHNodeGPULayout() {
            std::cout << "\n=== BVHNodeGPU Memory Layout ===" << std::endl;
            std::cout << "Size: " << sizeof(BVHNodeGPU) << " bytes" << std::endl;
            std::cout << "Alignment: " << alignof(BVHNodeGPU) << " bytes" << std::endl;
            std::cout << "Offsets:" << std::endl;
            std::cout << "  aabbMin:        " << offsetof(BVHNodeGPU, aabbMin) << std::endl;
            std::cout << "  aabbMax:        " << offsetof(BVHNodeGPU, aabbMax) << std::endl;
            std::cout << "  leftChild:      " << offsetof(BVHNodeGPU, leftChild) << std::endl;
            std::cout << "  rightChild:     " << offsetof(BVHNodeGPU, rightChild) << std::endl;
            std::cout << "  triangleIndex:  " << offsetof(BVHNodeGPU, triangleIndex) << std::endl;
            std::cout << "  triangleCount:  " << offsetof(BVHNodeGPU, triangleCount) << std::endl;
            std::cout << "================================\n" << std::endl;
        }

        inline void printBufferInfo(const char* name, size_t elementSize, size_t elementCount) {
            size_t totalBytes = elementSize * elementCount;
            std::cout << "\n=== SSBO Buffer Info: " << name << " ===" << std::endl;
            std::cout << "Element Size:  " << elementSize << " bytes" << std::endl;
            std::cout << "Element Count: " << elementCount << std::endl;
            std::cout << "Total Size:    " << totalBytes << " bytes (" 
                      << (totalBytes / 1024.0f) << " KB)" << std::endl;
            std::cout << "================================\n" << std::endl;
        }
    }

} // namespace GPUStructs

/**
 * ============================================================================
 * INTEGRATION NOTES FOR FUTURE PHASES
 * ============================================================================
 * 
 * Phase 3 - Shader Integration:
 * ------------------------------
 * GLSL shader buffer declarations should match these layouts exactly:
 * 
 * layout(std430, binding = X) buffer TriangleBuffer {
 *     TriangleGPU triangles[];
 * };
 * 
 * layout(std430, binding = Y) buffer BVHBuffer {
 *     BVHNodeGPU nodes[];
 * };
 * 
 * Phase 4 - Scene Serialization:
 * -------------------------------
 * Conversion from host structures (Triangle, BVHNode) to GPU structures:
 * 
 * std::vector<GPUStructs::TriangleGPU> serializeTriangles(const std::vector<Triangle>& triangles) {
 *     std::vector<GPUStructs::TriangleGPU> gpuTriangles;
 *     gpuTriangles.reserve(triangles.size());
 *     for (const auto& tri : triangles) {
 *         gpuTriangles.emplace_back(tri.v0, tri.v1, tri.v2, tri.normal, tri.materialIndex);
 *     }
 *     return gpuTriangles;
 * }
 * 
 * Phase 5 - Upload Pipeline:
 * ---------------------------
 * Upload to GPU memory via SSBO:
 * 
 * GLuint triangleSSBO;
 * glGenBuffers(1, &triangleSSBO);
 * glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
 * glBufferData(GL_SHADER_STORAGE_BUFFER, 
 *              gpuTriangles.size() * sizeof(GPUStructs::TriangleGPU),
 *              gpuTriangles.data(), GL_STATIC_DRAW);
 * glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_POINT, triangleSSBO);
 * 
 * Phase 6 - Validation:
 * ---------------------
 * Always validate uploads:
 * - Check glGetError() after buffer operations
 * - Verify buffer sizes match expected values
 * - Use Debug::printBufferInfo() for diagnostics
 * - Consider buffer orphaning for dynamic updates (glBufferData with NULL, then glBufferSubData)
 * 
 * Performance Considerations:
 * ---------------------------
 * - Use GL_STATIC_DRAW for static geometry
 * - Use GL_DYNAMIC_DRAW for animated/procedural geometry
 * - Consider persistent mapped buffers (GL_ARB_buffer_storage) for streaming
 * - Batch small updates to avoid pipeline stalls
 * - Profile with GPU tools (NSight, RenderDoc) to identify bottlenecks
 * ============================================================================
 */
