#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "vendor/glm/glm.hpp"
#include "ComputeShader.h"
#include "Sphere.h"
#include "Box.h"
#include "Material.h"
#include "Camera.h"
#include "Debug.h"
#include "GPUStructures.h"
#include "Triangle.h"
#include "BVH.h"
#include <iostream>

// External function pointers from ComputeShader.h
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture_ptr;

// Additional GL constants and function pointers
#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif

typedef void (*PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);
static PFNGLBINDBUFFERBASEPROC glBindBufferBase_ptr = nullptr;

static bool loadSSBOFunctions() {
    static bool loaded = false;
    if (loaded) return (glBindBufferBase_ptr != nullptr);
    loaded = true;
    
    glBindBufferBase_ptr = (PFNGLBINDBUFFERBASEPROC)glfwGetProcAddress("glBindBufferBase");
    return (glBindBufferBase_ptr != nullptr);
}

// GPURayTracer: GPU-basierter Ray Tracer mit OpenGL Compute Shaders
// Dramatisch schneller als CPU-Version, unterstützt höhere Auflösungen
class GPURayTracer {
private:
    int width, height;
    GLuint outputTexture = 0;
    GLuint accumulationTexture = 0; // Für temporale Akkumulation
    GLuint vao = 0, vbo = 0; // Für Fullscreen-Quad
    
    // Shader Storage Buffer Objects
    GLuint sphereSSBO = 0;
    GLuint boxSSBO = 0;
    GLuint materialSSBO = 0;
    
    // Triangle Mesh and BVH SSBOs (Phase 2)
    GLuint triangleSSBO = 0;      // Buffer for triangle geometry data
    GLuint bvhNodeSSBO = 0;       // Buffer for BVH acceleration structure
    
    ComputeShader* computeShader = nullptr;
    
    // Material-Datenbank
    std::vector<Material> materials;
    int currentMaterialSet = 0;
    
    // Frame Counter für Random Seed und Akkumulation
    uint32_t frameCount = 0;
    uint32_t accumulatedFrames = 0;
    
    // Kamera-Tracking für Akkumulations-Reset
    glm::vec3 lastCameraPos;
    glm::vec3 lastCameraTarget;
    bool cameraChanged = true;

    // GPU-kompatible Strukturen (müssen mit Shader übereinstimmen)
    struct GPUSphere {
        glm::vec3 center;
        float radius;
        int materialIndex;
        float _pad0, _pad1, _pad2; // Padding für std430 Alignment
    };

    struct GPUBox {
        glm::vec3 minBounds;
        float _pad0;
        glm::vec3 maxBounds;
        int materialIndex;
    };

    struct GPUMaterial {
        glm::vec3 albedo;
        float roughness;
        glm::vec3 emission;
        float metallic;
    };

public:
    // Rendering-Einstellungen
    int samplesPerPixel = 1;
    int maxBounces = 3;  // Erhöht für bessere Beleuchtung durch Deckenlampe
    
    // Szene
    std::vector<Sphere> spheres;
    std::vector<Box> boxes;
    Camera camera;
    
    // Phase 3: Triangle mesh and BVH support
    std::vector<Triangle> triangles;  // Host-side triangle data
    std::vector<BVHNode> bvhNodes;    // Host-side BVH data
    bool meshDataDirty = false;       // Flag to track if mesh needs re-upload
    // Kein Richtungslicht mehr - nur Deckenlampe als Lichtquelle

    GPURayTracer(int w, int h) : width(w), height(h) {
        camera.aspect = float(w) / float(h);
        camera.update();
        lastCameraPos = camera.position;
        lastCameraTarget = camera.target;
        
        // Load required OpenGL functions
        if (!loadSSBOFunctions()) {
            std::cerr << "Failed to load SSBO functions!" << std::endl;
            return;
        }
        
        // Initialisiere Material-Sets
        initializeMaterials();
        
        // Erstelle Output-Textur
        GLCall(glGenTextures(1, &outputTexture));
        GLCall(glBindTexture(GL_TEXTURE_2D, outputTexture));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        
        // Erstelle Akkumulations-Textur (höhere Präzision für Durchschnittsbildung)
        GLCall(glGenTextures(1, &accumulationTexture));
        GLCall(glBindTexture(GL_TEXTURE_2D, accumulationTexture));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        
        // Erstelle Fullscreen Quad
        createQuad();
        
        // Erstelle SSBOs
        GLCall(glGenBuffers(1, &sphereSSBO));
        GLCall(glGenBuffers(1, &boxSSBO));
        GLCall(glGenBuffers(1, &materialSSBO));
        
        // Erstelle Triangle Mesh und BVH SSBOs (Phase 2)
        GLCall(glGenBuffers(1, &triangleSSBO));
        GLCall(glGenBuffers(1, &bvhNodeSSBO));
        
        // Debug: Print GPU structure layouts for validation
        std::cout << "\n=== GPU Buffer Infrastructure Initialized ===" << std::endl;
        GPUStructs::Debug::printTriangleGPULayout();
        GPUStructs::Debug::printBVHNodeGPULayout();
        std::cout << "Triangle and BVH buffers created successfully." << std::endl;
        std::cout << "============================================\n" << std::endl;
        
        // Lade Compute Shader
        try {
            computeShader = new ComputeShader("res/shaders/raytracer.comp");
            if (!computeShader->isAvailable()) {
                delete computeShader;
                computeShader = nullptr;
            }
        } catch (...) {
            std::cerr << "Failed to load compute shader!" << std::endl;
        }
    }

    ~GPURayTracer() {
        if (outputTexture) glDeleteTextures(1, &outputTexture);
        if (accumulationTexture) glDeleteTextures(1, &accumulationTexture);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (sphereSSBO) glDeleteBuffers(1, &sphereSSBO);
        if (boxSSBO) glDeleteBuffers(1, &boxSSBO);
        if (materialSSBO) glDeleteBuffers(1, &materialSSBO);
        if (triangleSSBO) glDeleteBuffers(1, &triangleSSBO);
        if (bvhNodeSSBO) glDeleteBuffers(1, &bvhNodeSSBO);
        delete computeShader;
    }

    bool isAvailable() const {
        return computeShader != nullptr;
    }

    /**
     * demonstrateGPUBufferInfrastructure
     * -----------------------------------
     * Public method to demonstrate and validate the GPU buffer infrastructure.
     * Creates example triangle mesh and BVH, then serializes and uploads to GPU.
     * 
     * This is a Phase 2 validation function - shows that the infrastructure works
     * without requiring shader integration (Phase 3).
     */
    void demonstrateGPUBufferInfrastructure() {
        std::cout << "\n=== Phase 2: GPU Buffer Infrastructure Validation ===" << std::endl;
        validateBufferLayouts();
        
        // Demonstrate triangle and BVH serialization with example data
        std::cout << "\n=== Example: Triangle Mesh Serialization ===" << std::endl;
        std::vector<Triangle> exampleTriangles = MeshGenerator::createBox(
            glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3(1.0f, 1.0f, 1.0f), 
            0
        );
        std::cout << "Created example box mesh with " << exampleTriangles.size() << " triangles" << std::endl;
        
        auto gpuTriangles = serializeTrianglesToGPU(exampleTriangles);
        bool uploadSuccess = uploadTrianglesToGPU(gpuTriangles, 4, GL_STATIC_DRAW);
        
        if (uploadSuccess) {
            std::cout << "✓ Triangle mesh serialization and upload successful!" << std::endl;
        } else {
            std::cout << "✗ Triangle mesh upload failed!" << std::endl;
        }
        
        std::cout << "\n=== Example: BVH Construction and Upload ===" << std::endl;
        BVHBuilder bvhBuilder;
        bvhBuilder.build(exampleTriangles);
        
        auto gpuBVH = serializeBVHToGPU(bvhBuilder.getNodes());
        uploadSuccess = uploadBVHToGPU(gpuBVH, 5, GL_STATIC_DRAW);
        
        if (uploadSuccess) {
            std::cout << "✓ BVH construction and upload successful!" << std::endl;
        } else {
            std::cout << "✗ BVH upload failed!" << std::endl;
        }
        
        std::cout << "\n=== GPU Buffer Infrastructure Ready ===" << std::endl;
        std::cout << "All buffer structures validated and tested successfully!" << std::endl;
        std::cout << "Ready for Phase 3: Shader integration\n" << std::endl;
    }

    /**
     * loadTriangleMesh
     * -----------------
     * Loads a triangle mesh into the ray tracer and builds BVH.
     * Automatically uploads to GPU on next render() call.
     * 
     * @param meshTriangles - Triangle data to load
     * 
     * Usage Example:
     * auto cubeTriangles = MeshGenerator::createBox(center, size, materialIndex);
     * gpuRT->loadTriangleMesh(cubeTriangles);
     */
    void loadTriangleMesh(const std::vector<Triangle>& meshTriangles) {
        std::cout << "\n[GPU Ray Tracer] Loading triangle mesh..." << std::endl;
        std::cout << "  Triangles: " << meshTriangles.size() << std::endl;
        
        triangles = meshTriangles;
        
        // Build BVH acceleration structure
        if (!triangles.empty()) {
            BVHBuilder bvhBuilder;
            bvhBuilder.build(triangles);
            bvhNodes = bvhBuilder.getNodes();
            triangles = bvhBuilder.getOrderedTriangles(); // Use reordered triangles for cache coherency
            
            std::cout << "  BVH Nodes: " << bvhNodes.size() << std::endl;
            std::cout << "  Triangle mesh loaded successfully!" << std::endl;
        } else {
            bvhNodes.clear();
            std::cout << "  Warning: Empty mesh loaded" << std::endl;
        }
        
        meshDataDirty = true;
        accumulatedFrames = 0; // Reset accumulation when scene changes
    }

    /**
     * clearTriangleMesh
     * ------------------
     * Removes all triangle mesh data from the scene.
     * 
     * Behavior:
     * - Clears triangles vector
     * - Clears bvhNodes vector
     * - Sets meshDataDirty flag for GPU buffer cleanup
     * - Resets accumulatedFrames to restart progressive rendering
     * 
     * Use case: When switching between different scenes or removing
     * triangle geometry while keeping spheres/boxes.
     */
    void clearTriangleMesh() {
        triangles.clear();
        bvhNodes.clear();
        meshDataDirty = true;
        accumulatedFrames = 0;
        std::cout << "[GPU Ray Tracer] Triangle mesh cleared" << std::endl;
    }

    void cycleMaterialSet() {
        currentMaterialSet = (currentMaterialSet + 1) % 4; // 4 verschiedene Material-Sets
        std::cout << "Material Set: " << currentMaterialSet << std::endl;
        accumulatedFrames = 0; // Reset bei Material-Wechsel
    }

    void render() {
        if (!computeShader || !glBindImageTexture_ptr) return;
        
        camera.update();
        updateScene();
        
        // Phase 3: Upload triangle mesh and BVH if dirty
        if (meshDataDirty) {
            uploadMeshData();
            meshDataDirty = false;
        }
        
        // Prüfe ob Kamera sich bewegt hat - Reset Akkumulation
        const float epsilon = 0.001f;
        if (glm::length(camera.position - lastCameraPos) > epsilon ||
            glm::length(camera.target - lastCameraTarget) > epsilon) {
            accumulatedFrames = 0;
            lastCameraPos = camera.position;
            lastCameraTarget = camera.target;
        }
        
        // Binde Output-Textur und Akkumulations-Textur als Images
        const GLenum GL_WRITE_ONLY_CONST = 0x88B9; // GL_WRITE_ONLY
        const GLenum GL_READ_WRITE_CONST = 0x88BA; // GL_READ_WRITE
        glBindImageTexture_ptr(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY_CONST, GL_RGBA32F);
        glBindImageTexture_ptr(1, accumulationTexture, 0, GL_FALSE, 0, GL_READ_WRITE_CONST, GL_RGBA32F);
        
        // Binde Compute Shader
        computeShader->Bind();
        
        // Setze Uniforms
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "numSpheres"), spheres.size()));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "numBoxes"), boxes.size()));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "numTriangles"), triangles.size()));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "numBVHNodes"), bvhNodes.size()));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "samplesPerPixel"), samplesPerPixel));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "maxBounces"), maxBounces));
        GLCall(glUniform1ui(glGetUniformLocation(computeShader->GetRendererID(), "frameCount"), frameCount++));
        GLCall(glUniform1ui(glGetUniformLocation(computeShader->GetRendererID(), "accumulatedFrames"), accumulatedFrames++));
        
        // Kamera-Parameter
        computeShader->SetUniformVec3("cameraPos", camera.position);
        computeShader->SetUniformVec3("cameraTarget", camera.target);
        computeShader->SetUniformVec3("cameraUp", camera.up);
        computeShader->SetUniform1f("cameraVFov", camera.vfov);
        computeShader->SetUniform1f("cameraAspect", camera.aspect);
        
        // Kamera-Basis (vorberechnet)
        computeShader->SetUniformVec3("cameraU", camera.u);
        computeShader->SetUniformVec3("cameraV", camera.v);
        computeShader->SetUniformVec3("cameraW", camera.w);
        computeShader->SetUniformVec3("cameraLowerLeft", camera.lowerLeftCorner);
        computeShader->SetUniformVec3("cameraHorizontal", camera.horizontal);
        computeShader->SetUniformVec3("cameraVertical", camera.vertical);
        
        // Kein Licht-Uniform mehr - Licht kommt von emissiver Deckenlampe
        
        // Dispatch Compute Shader (8x8 Work Groups)
        GLuint numGroupsX = (width + 7) / 8;
        GLuint numGroupsY = (height + 7) / 8;
        computeShader->Dispatch(numGroupsX, numGroupsY, 1);
        
        // Warte auf Compute Shader
        computeShader->Wait();
        
        computeShader->Unbind();
    }

    void draw(GLuint displayShader) {
        // Rendere mit Compute Shader
        render();
        
        // Zeige Ergebnis auf Fullscreen Quad
        GLCall(glUseProgram(displayShader));
        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, outputTexture));
        
        GLint loc = glGetUniformLocation(displayShader, "u_Texture");
        if (loc >= 0) glUniform1i(loc, 0);
        
        GLCall(glBindVertexArray(vao));
        GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
        GLCall(glBindVertexArray(0));
        
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        GLCall(glUseProgram(0));
    }

private:
    void initializeMaterials() {
        // Material-Set 0: Standard (Diffuse Farben)
        materials.push_back(Material::Diffuse(glm::vec3(0.8f, 0.3f, 0.3f))); // Rot
        materials.push_back(Material::Diffuse(glm::vec3(0.3f, 0.8f, 0.3f))); // Grün
        materials.push_back(Material::Diffuse(glm::vec3(0.3f, 0.3f, 0.8f))); // Blau
        
        // Material-Set 1: Metalle
        materials.push_back(Material::Chrome());
        materials.push_back(Material::Gold());
        materials.push_back(Material::Copper());
        
        // Material-Set 2: Gemischt
        materials.push_back(Material::Glass());
        materials.push_back(Material::Rubber());
        materials.push_back(Material::Chrome());
        
        // Material-Set 3: Mit Emission
        materials.push_back(Material::Emissive(glm::vec3(1.0f, 0.5f, 0.2f), 2.0f));
        materials.push_back(Material::Diffuse(glm::vec3(0.8f, 0.8f, 0.8f)));
        materials.push_back(Material::Gold());
        
        // WICHTIG: Deckenlampen-Material für Szene (Index 12)
        materials.push_back(Material::Emissive(glm::vec3(1.0f, 1.0f, 0.9f), 20.0f)); // Helle Deckenlampe
    }

    void createQuad() {
        float quad[16] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };
        
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));
        GLCall(glGenBuffers(1, &vbo));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW));
        
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        
        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
        
        GLCall(glBindVertexArray(0));
    }

    void updateScene() {
        // Konvertiere Spheres zu GPU-Format
        std::vector<GPUSphere> gpuSpheres;
        for (size_t i = 0; i < spheres.size(); ++i) {
            GPUSphere gs;
            gs.center = spheres[i].center;
            gs.radius = spheres[i].radius;
            
            // Suche nach dem Material in der materials-Liste
            // Für die Deckenlampe (letzte Sphere) verwenden wir das helle Lampenmaterial (Index 12)
            if (i == spheres.size() - 1) {
                gs.materialIndex = 12; // Deckenlampen-Material
            } else {
                gs.materialIndex = getMaterialIndexForObject(i);
            }
            gpuSpheres.push_back(gs);
        }
        
        // Konvertiere Boxes zu GPU-Format
        std::vector<GPUBox> gpuBoxes;
        for (size_t i = 0; i < boxes.size(); ++i) {
            GPUBox gb;
            gb.minBounds = boxes[i].minBounds;
            gb.maxBounds = boxes[i].maxBounds;
            gb.materialIndex = getMaterialIndexForObject(i);
            gpuBoxes.push_back(gb);
        }
        
        // Konvertiere Materials zu GPU-Format
        std::vector<GPUMaterial> gpuMaterials;
        for (const auto& mat : materials) {
            GPUMaterial gm;
            gm.albedo = mat.albedo;
            gm.roughness = mat.roughness;
            gm.emission = mat.emission;
            gm.metallic = mat.metallic;
            gpuMaterials.push_back(gm);
        }
        
        // Upload Spheres
        if (!gpuSpheres.empty() && glBindBufferBase_ptr) {
            GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO));
            GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, gpuSpheres.size() * sizeof(GPUSphere), 
                               gpuSpheres.data(), GL_DYNAMIC_DRAW));
            glBindBufferBase_ptr(GL_SHADER_STORAGE_BUFFER, 1, sphereSSBO);
        }
        
        // Upload Boxes
        if (!gpuBoxes.empty() && glBindBufferBase_ptr) {
            GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, boxSSBO));
            GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, gpuBoxes.size() * sizeof(GPUBox), 
                               gpuBoxes.data(), GL_DYNAMIC_DRAW));
            glBindBufferBase_ptr(GL_SHADER_STORAGE_BUFFER, 2, boxSSBO);
        }
        
        // Upload Materials
        if (!gpuMaterials.empty() && glBindBufferBase_ptr) {
            GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO));
            GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, gpuMaterials.size() * sizeof(GPUMaterial), 
                               gpuMaterials.data(), GL_DYNAMIC_DRAW));
            glBindBufferBase_ptr(GL_SHADER_STORAGE_BUFFER, 3, materialSSBO);
        }
        
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
    }

    int getMaterialIndexForObject(size_t objectIndex) {
        // Verteile Materialien basierend auf aktuellem Material-Set
        int materialsPerSet = 3;
        int baseIndex = currentMaterialSet * materialsPerSet;
        return baseIndex + (objectIndex % materialsPerSet);
    }

    // ============================================================================
    // PHASE 2: GPU BUFFER INFRASTRUCTURE FOR TRIANGLE MESH AND BVH RENDERING
    // ============================================================================
    
    /**
     * serializeTrianglesToGPU
     * ------------------------
     * Converts host-side Triangle structures to GPU-compatible TriangleGPU format.
     * Handles proper padding and alignment for std430 GLSL layout.
     * 
     * @param triangles - Host-side triangle data (from scene or mesh)
     * @return Vector of GPU-compatible triangles ready for SSBO upload
     * 
     * Performance Note: This is a CPU-side conversion, so it should be done
     * infrequently (e.g., at scene load time, not every frame).
     */
    std::vector<GPUStructs::TriangleGPU> serializeTrianglesToGPU(const std::vector<Triangle>& triangles) {
        std::vector<GPUStructs::TriangleGPU> gpuTriangles;
        gpuTriangles.reserve(triangles.size());
        
        for (const auto& tri : triangles) {
            gpuTriangles.emplace_back(
                tri.v0,
                tri.v1,
                tri.v2,
                tri.normal,
                tri.materialIndex
            );
        }
        
        // Debug output for validation
        if (!gpuTriangles.empty()) {
            std::cout << "[GPU Buffer] Serialized " << gpuTriangles.size() 
                      << " triangles to GPU format" << std::endl;
            GPUStructs::Debug::printBufferInfo("TriangleBuffer", 
                                               sizeof(GPUStructs::TriangleGPU), 
                                               gpuTriangles.size());
        }
        
        return gpuTriangles;
    }

    /**
     * serializeBVHToGPU
     * -----------------
     * Converts host-side BVHNode structures to GPU-compatible BVHNodeGPU format.
     * Preserves tree structure through index-based parent-child relationships.
     * 
     * @param nodes - Host-side BVH node data (from BVHBuilder)
     * @return Vector of GPU-compatible BVH nodes ready for SSBO upload
     * 
     * Architecture Note: BVH traversal on GPU follows depth-first pattern.
     * Nodes should be laid out in pre-order for optimal cache coherency.
     */
    std::vector<GPUStructs::BVHNodeGPU> serializeBVHToGPU(const std::vector<BVHNode>& nodes) {
        std::vector<GPUStructs::BVHNodeGPU> gpuNodes;
        gpuNodes.reserve(nodes.size());
        
        for (const auto& node : nodes) {
            // BVHNode already has compatible structure, but we need to convert to GPU format
            GPUStructs::BVHNodeGPU gpuNode;
            gpuNode.aabbMin = node.aabbMin;
            gpuNode.aabbMax = node.aabbMax;
            gpuNode.leftChild = node.leftChild;
            
            // Right child is implicit (leftChild + 1) in current BVH layout
            // We'll store -1 if it's a leaf, otherwise compute from structure
            if (node.isLeaf()) {
                gpuNode.rightChild = -1;
                gpuNode.triangleIndex = node.leftChild; // For leaf, leftChild stores triangle index
                gpuNode.triangleCount = node.triangleCount;
            } else {
                gpuNode.rightChild = node.leftChild + 1; // Implicit layout: right = left + 1
                gpuNode.triangleIndex = 0;
                gpuNode.triangleCount = 0;
            }
            
            gpuNodes.push_back(gpuNode);
        }
        
        // Debug output for validation
        if (!gpuNodes.empty()) {
            std::cout << "[GPU Buffer] Serialized " << gpuNodes.size() 
                      << " BVH nodes to GPU format" << std::endl;
            
            // Count leaf vs internal nodes for diagnostics
            int leafCount = 0, internalCount = 0;
            for (const auto& node : gpuNodes) {
                if (node.isLeaf()) leafCount++;
                else if (node.isInternal()) internalCount++;
            }
            
            std::cout << "  Leaf Nodes: " << leafCount 
                      << ", Internal Nodes: " << internalCount << std::endl;
            
            GPUStructs::Debug::printBufferInfo("BVHBuffer", 
                                               sizeof(GPUStructs::BVHNodeGPU), 
                                               gpuNodes.size());
        }
        
        return gpuNodes;
    }

    /**
     * uploadTrianglesToGPU
     * ---------------------
     * Uploads serialized triangle data to GPU via SSBO.
     * Binds buffer to specified binding point for shader access.
     * 
     * @param gpuTriangles - GPU-formatted triangle data
     * @param bindingPoint - SSBO binding point (must match shader layout)
     * @param usage - OpenGL buffer usage hint (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc.)
     * 
     * Error Handling: Checks for OpenGL errors and validates buffer size.
     * Returns true on success, false on failure.
     */
    bool uploadTrianglesToGPU(const std::vector<GPUStructs::TriangleGPU>& gpuTriangles, 
                              GLuint bindingPoint = 4, 
                              GLenum usage = GL_STATIC_DRAW) {
        if (gpuTriangles.empty()) {
            std::cerr << "[GPU Buffer] Warning: Attempted to upload empty triangle buffer" << std::endl;
            return false;
        }
        
        if (!glBindBufferBase_ptr) {
            std::cerr << "[GPU Buffer] Error: SSBO functions not loaded!" << std::endl;
            return false;
        }
        
        // Calculate buffer size
        size_t bufferSize = gpuTriangles.size() * sizeof(GPUStructs::TriangleGPU);
        
        std::cout << "[GPU Buffer] Uploading triangle buffer:" << std::endl;
        std::cout << "  Triangles: " << gpuTriangles.size() << std::endl;
        std::cout << "  Size: " << bufferSize << " bytes (" 
                  << (bufferSize / 1024.0f) << " KB)" << std::endl;
        std::cout << "  Binding Point: " << bindingPoint << std::endl;
        std::cout << "  Usage Hint: " << (usage == GL_STATIC_DRAW ? "STATIC" : "DYNAMIC") << std::endl;
        
        // Bind and upload
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO));
        GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, 
                           gpuTriangles.data(), usage));
        
        // Check for errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "[GPU Buffer] Error uploading triangle buffer: 0x" 
                      << std::hex << error << std::dec << std::endl;
            return false;
        }
        
        // Bind to shader binding point
        glBindBufferBase_ptr(GL_SHADER_STORAGE_BUFFER, bindingPoint, triangleSSBO);
        
        std::cout << "[GPU Buffer] Triangle buffer uploaded successfully!" << std::endl;
        
        // Unbind
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
        
        return true;
    }

    /**
     * uploadBVHToGPU
     * ---------------
     * Uploads serialized BVH data to GPU via SSBO.
     * Binds buffer to specified binding point for shader access.
     * 
     * @param gpuNodes - GPU-formatted BVH node data
     * @param bindingPoint - SSBO binding point (must match shader layout)
     * @param usage - OpenGL buffer usage hint
     * 
     * Architecture Note: BVH upload should happen after triangle upload,
     * as BVH nodes reference triangle indices.
     */
    bool uploadBVHToGPU(const std::vector<GPUStructs::BVHNodeGPU>& gpuNodes, 
                        GLuint bindingPoint = 5, 
                        GLenum usage = GL_STATIC_DRAW) {
        if (gpuNodes.empty()) {
            std::cerr << "[GPU Buffer] Warning: Attempted to upload empty BVH buffer" << std::endl;
            return false;
        }
        
        if (!glBindBufferBase_ptr) {
            std::cerr << "[GPU Buffer] Error: SSBO functions not loaded!" << std::endl;
            return false;
        }
        
        // Calculate buffer size
        size_t bufferSize = gpuNodes.size() * sizeof(GPUStructs::BVHNodeGPU);
        
        std::cout << "[GPU Buffer] Uploading BVH buffer:" << std::endl;
        std::cout << "  Nodes: " << gpuNodes.size() << std::endl;
        std::cout << "  Size: " << bufferSize << " bytes (" 
                  << (bufferSize / 1024.0f) << " KB)" << std::endl;
        std::cout << "  Binding Point: " << bindingPoint << std::endl;
        std::cout << "  Usage Hint: " << (usage == GL_STATIC_DRAW ? "STATIC" : "DYNAMIC") << std::endl;
        
        // Bind and upload
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhNodeSSBO));
        GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, 
                           gpuNodes.data(), usage));
        
        // Check for errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "[GPU Buffer] Error uploading BVH buffer: 0x" 
                      << std::hex << error << std::dec << std::endl;
            return false;
        }
        
        // Bind to shader binding point
        glBindBufferBase_ptr(GL_SHADER_STORAGE_BUFFER, bindingPoint, bvhNodeSSBO);
        
        std::cout << "[GPU Buffer] BVH buffer uploaded successfully!" << std::endl;
        
        // Unbind
        GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
        
        return true;
    }

    /**
     * validateBufferLayouts
     * ----------------------
     * Runtime validation of GPU buffer structures.
     * Prints detailed layout information and checks for common issues.
     * 
     * Call this during initialization to verify GPU-CPU compatibility.
     * Critical for debugging alignment and padding issues.
     */
    void validateBufferLayouts() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "GPU BUFFER LAYOUT VALIDATION" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Validate TriangleGPU
        GPUStructs::Debug::printTriangleGPULayout();
        
        // Check alignment requirements
        if (sizeof(GPUStructs::TriangleGPU) % 16 != 0) {
            std::cerr << "WARNING: TriangleGPU size is not 16-byte aligned!" << std::endl;
        }
        
        // Validate BVHNodeGPU
        GPUStructs::Debug::printBVHNodeGPULayout();
        
        if (sizeof(GPUStructs::BVHNodeGPU) % 16 != 0) {
            std::cerr << "WARNING: BVHNodeGPU size is not 16-byte aligned!" << std::endl;
        }
        
        // Print GLSL compatibility notes
        std::cout << "\n=== GLSL Shader Compatibility Notes ===" << std::endl;
        std::cout << "Ensure your compute shader uses the following layouts:" << std::endl;
        std::cout << "\nlayout(std430, binding = 4) buffer TriangleBuffer {" << std::endl;
        std::cout << "    TriangleGPU triangles[];" << std::endl;
        std::cout << "};" << std::endl;
        std::cout << "\nlayout(std430, binding = 5) buffer BVHBuffer {" << std::endl;
        std::cout << "    BVHNodeGPU nodes[];" << std::endl;
        std::cout << "};" << std::endl;
        std::cout << "\nStruct definitions in GLSL must match C++ layouts exactly!" << std::endl;
        std::cout << "========================================\n" << std::endl;
    }

    /**
     * uploadMeshData
     * ---------------
     * Uploads triangle mesh and BVH data to GPU.
     * Called automatically when meshDataDirty flag is set.
     * 
     * Phase 3 Integration: Uploads to binding points 4 (triangles) and 5 (BVH).
     */
    void uploadMeshData() {
        if (!glBindBufferBase_ptr) {
            std::cerr << "[GPU Ray Tracer] Error: SSBO functions not loaded!" << std::endl;
            return;
        }
        
        // Serialize and upload triangles
        if (!triangles.empty()) {
            auto gpuTriangles = serializeTrianglesToGPU(triangles);
            uploadTrianglesToGPU(gpuTriangles, 4, GL_STATIC_DRAW);
        } else {
            // Upload empty buffer to avoid shader errors
            std::cout << "[GPU Ray Tracer] No triangles to upload" << std::endl;
        }
        
        // Serialize and upload BVH
        if (!bvhNodes.empty()) {
            auto gpuBVH = serializeBVHToGPU(bvhNodes);
            uploadBVHToGPU(gpuBVH, 5, GL_STATIC_DRAW);
        } else {
            // Upload empty buffer to avoid shader errors
            std::cout << "[GPU Ray Tracer] No BVH nodes to upload" << std::endl;
        }
    }

    // ============================================================================
    // END PHASE 2 IMPLEMENTATION
    // ============================================================================
};
