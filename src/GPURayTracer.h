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
    GLuint vao = 0, vbo = 0; // Für Fullscreen-Quad
    
    // Shader Storage Buffer Objects
    GLuint sphereSSBO = 0;
    GLuint boxSSBO = 0;
    GLuint materialSSBO = 0;
    
    ComputeShader* computeShader = nullptr;
    
    // Material-Datenbank
    std::vector<Material> materials;
    int currentMaterialSet = 0;
    
    // Frame Counter für Random Seed
    uint32_t frameCount = 0;

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
    // Kein Richtungslicht mehr - nur Deckenlampe als Lichtquelle

    GPURayTracer(int w, int h) : width(w), height(h) {
        camera.aspect = float(w) / float(h);
        camera.update();
        
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
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        
        // Erstelle Fullscreen Quad
        createQuad();
        
        // Erstelle SSBOs
        GLCall(glGenBuffers(1, &sphereSSBO));
        GLCall(glGenBuffers(1, &boxSSBO));
        GLCall(glGenBuffers(1, &materialSSBO));
        
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
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (sphereSSBO) glDeleteBuffers(1, &sphereSSBO);
        if (boxSSBO) glDeleteBuffers(1, &boxSSBO);
        if (materialSSBO) glDeleteBuffers(1, &materialSSBO);
        delete computeShader;
    }

    bool isAvailable() const {
        return computeShader != nullptr;
    }

    void cycleMaterialSet() {
        currentMaterialSet = (currentMaterialSet + 1) % 4; // 4 verschiedene Material-Sets
        std::cout << "Material Set: " << currentMaterialSet << std::endl;
    }

    void render() {
        if (!computeShader || !glBindImageTexture_ptr) return;
        
        camera.update();
        updateScene();
        
        // Binde Output-Textur als Image
        const GLenum GL_WRITE_ONLY_CONST = 0x88B9; // GL_WRITE_ONLY
        glBindImageTexture_ptr(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY_CONST, GL_RGBA8);
        
        // Binde Compute Shader
        computeShader->Bind();
        
        // Setze Uniforms
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "numSpheres"), spheres.size()));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "numBoxes"), boxes.size()));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "samplesPerPixel"), samplesPerPixel));
        GLCall(glUniform1i(glGetUniformLocation(computeShader->GetRendererID(), "maxBounces"), maxBounces));
        GLCall(glUniform1ui(glGetUniformLocation(computeShader->GetRendererID(), "frameCount"), frameCount++));
        
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
            gs.materialIndex = getMaterialIndexForObject(i);
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
};
