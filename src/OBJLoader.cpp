#include "OBJLoader.h"
#include "Texture.h"
#include <obj/objparser.h>
#include <obj/mtlparser.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Helper to resolve file paths relative to a base directory
static std::string ResolvePath(const std::string& basePath, const std::string& filename) {
    // Extract directory from base path
    size_t lastSlash = basePath.find_last_of("/\\");
    std::string baseDir = (lastSlash != std::string::npos) ? basePath.substr(0, lastSlash + 1) : "";
    
    // Handle absolute Windows paths (e.g., D:\path\to\file.png)
    if (filename.length() >= 3 && filename[1] == ':' && (filename[2] == '\\' || filename[2] == '/')) {
        std::cerr << "[OBJLoader] Warning: Absolute Windows path detected: " << filename << std::endl;
        std::cerr << "[OBJLoader]   This will likely fail. Please use relative paths in MTL files." << std::endl;
        return filename;
    }
    
    // Convert backslashes to forward slashes for cross-platform compatibility
    std::string normalizedFilename = filename;
    std::replace(normalizedFilename.begin(), normalizedFilename.end(), '\\', '/');
    
    return baseDir + normalizedFilename;
}

// Helper to load MTL file
static bool LoadMTL(const std::string& mtlPath, std::map<std::string, Material>& materials) {
    std::cout << "[OBJLoader] Loading MTL file: " << mtlPath << std::endl;
    
    obj::mtlparser parser;
    Material* currentMaterial = nullptr;
    
    // Connect signal handlers
    parser.errorSignal.connect([](unsigned int line, const std::string& msg) {
        std::cerr << "[MTL Parser] Error line " << line << ": " << msg << std::endl;
    });
    
    parser.beginMaterialSignal.connect([&](const std::string& name) {
        materials[name] = Material(name);
        currentMaterial = &materials[name];
        std::cout << "[MTL] New material: " << name << std::endl;
    });
    
    parser.ambientSignal.connect([&](const obj::vec3d& color) {
        if (currentMaterial) {
            currentMaterial->ambient[0] = static_cast<float>(color.x);
            currentMaterial->ambient[1] = static_cast<float>(color.y);
            currentMaterial->ambient[2] = static_cast<float>(color.z);
        }
    });
    
    parser.diffuseSignal.connect([&](const obj::vec3d& color) {
        if (currentMaterial) {
            currentMaterial->diffuse[0] = static_cast<float>(color.x);
            currentMaterial->diffuse[1] = static_cast<float>(color.y);
            currentMaterial->diffuse[2] = static_cast<float>(color.z);
        }
    });
    
    parser.specularSignal.connect([&](const obj::vec3d& color) {
        if (currentMaterial) {
            currentMaterial->specular[0] = static_cast<float>(color.x);
            currentMaterial->specular[1] = static_cast<float>(color.y);
            currentMaterial->specular[2] = static_cast<float>(color.z);
        }
    });
    
    parser.specularExpSignal.connect([&](double exp) {
        if (currentMaterial) {
            currentMaterial->specularExponent = static_cast<float>(exp);
        }
    });
    
    parser.opacitySignal.connect([&](double opacity) {
        if (currentMaterial) {
            currentMaterial->opacity = static_cast<float>(opacity);
        }
    });
    
    parser.textureDiffuseSignal.connect([&](const std::string& texPath) {
        if (currentMaterial) {
            // Resolve texture path relative to MTL file
            std::string fullTexPath = ResolvePath(mtlPath, texPath);
            currentMaterial->diffuseTexturePath = fullTexPath;
            
            std::cout << "[MTL] map_Kd: " << texPath << " -> " << fullTexPath << std::endl;
            
            // Try to load the texture
            auto texture = std::make_shared<Texture>(fullTexPath);
            if (texture->IsValid()) {
                currentMaterial->diffuseTexture = texture;
                std::cout << "[MTL]   Texture loaded successfully!" << std::endl;
            } else {
                std::cerr << "[MTL]   Failed to load texture: " << fullTexPath << std::endl;
            }
        }
    });
    
    // Parse the MTL file
    parser.parse(mtlPath.c_str());
    
    std::cout << "[OBJLoader] Loaded " << materials.size() << " material(s) from MTL" << std::endl;
    return !materials.empty();
}

bool OBJLoader::LoadOBJ(const std::string& filepath, MeshData& outMesh) {
    std::cout << "[OBJLoader] Loading OBJ file: " << filepath << std::endl;
    
    obj::objparser parser;
    
    // Storage for parsed data
    std::vector<obj::vec3d> positions;
    std::vector<obj::vec3d> texcoords;
    std::vector<obj::vec3d> normals;
    
    // Temporary data for building final mesh
    struct Vertex {
        float x, y, z, u, v;
        float r, g, b; // Material diffuse color
        bool operator==(const Vertex& other) const {
            return x == other.x && y == other.y && z == other.z && 
                   u == other.u && v == other.v &&
                   r == other.r && g == other.g && b == other.b;
        }
    };
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::map<std::string, std::vector<unsigned int>> materialFaces;
    
    std::string currentMtl;
    std::vector<obj::face_index> currentFace;
    
    // Connect signal handlers
    parser.errorSignal.connect([](unsigned int line, const std::string& msg) {
        std::cerr << "[OBJ Parser] Error line " << line << ": " << msg << std::endl;
    });
    
    parser.vertexSignal.connect([&](const obj::vec3d& v) {
        positions.push_back(v);
    });
    
    parser.texcoordSignal.connect([&](const obj::vec3d& t) {
        texcoords.push_back(t);
    });
    
    parser.normalSignal.connect([&](const obj::vec3d& n) {
        normals.push_back(n);
    });
    
    parser.materialLibSignal.connect([&](const std::string& mtlFile) {
        std::string mtlPath = ResolvePath(filepath, mtlFile);
        std::cout << "[OBJ] mtllib: " << mtlFile << " -> " << mtlPath << std::endl;
        LoadMTL(mtlPath, outMesh.materials);
    });
    
    parser.materialUseSignal.connect([&](const std::string& mtlName) {
        currentMtl = mtlName;
        std::cout << "[OBJ] usemtl: " << mtlName << std::endl;
    });
    
    parser.faceBeginSignal.connect([&](unsigned int numElements) {
        currentFace.clear();
    });
    
    parser.faceElementSignal.connect([&](const obj::face_index& idx) {
        currentFace.push_back(idx);
    });
    
    parser.faceEndSignal.connect([&]() {
        if (currentFace.size() < 3) return;
        
        // Get current material's diffuse color (default to white if no material)
        float matR = 1.0f, matG = 1.0f, matB = 1.0f;
        if (!currentMtl.empty() && outMesh.materials.find(currentMtl) != outMesh.materials.end()) {
            const Material& mat = outMesh.materials[currentMtl];
            matR = mat.diffuse[0];
            matG = mat.diffuse[1];
            matB = mat.diffuse[2];
            outMesh.hasVertexColors = true;
        }
        
        // Triangulate the face (fan triangulation)
        for (size_t i = 1; i < currentFace.size() - 1; ++i) {
            // Triangle: v0, vi, vi+1
            for (int j : {0, (int)i, (int)i + 1}) {
                const auto& faceIdx = currentFace[j];
                
                Vertex v;
                // Position (required)
                if (faceIdx.vertexIdx > 0 && faceIdx.vertexIdx <= (int)positions.size()) {
                    const auto& pos = positions[faceIdx.vertexIdx - 1];
                    v.x = static_cast<float>(pos.x);
                    v.y = static_cast<float>(pos.y);
                    v.z = static_cast<float>(pos.z);
                } else {
                    std::cerr << "[OBJ] Invalid vertex index: " << faceIdx.vertexIdx << std::endl;
                    return;
                }
                
                // Texture coordinates (optional)
                v.u = 0.0f;
                v.v = 0.0f;
                if (faceIdx.texCoordIdx > 0 && faceIdx.texCoordIdx <= (int)texcoords.size()) {
                    const auto& tc = texcoords[faceIdx.texCoordIdx - 1];
                    v.u = static_cast<float>(tc.x);
                    v.v = static_cast<float>(tc.y);
                    outMesh.hasTexCoords = true;
                }
                
                // Material color (from current material)
                v.r = matR;
                v.g = matG;
                v.b = matB;
                
                // Find or add vertex
                auto it = std::find(vertices.begin(), vertices.end(), v);
                unsigned int vertexIndex;
                if (it != vertices.end()) {
                    vertexIndex = static_cast<unsigned int>(std::distance(vertices.begin(), it));
                } else {
                    vertexIndex = static_cast<unsigned int>(vertices.size());
                    vertices.push_back(v);
                }
                
                indices.push_back(vertexIndex);
            }
        }
    });
    
    // Parse the OBJ file
    parser.parse(filepath.c_str());
    
    if (vertices.empty() || indices.empty()) {
        std::cerr << "[OBJLoader] ERROR: OBJ file contains no geometry: " << filepath << std::endl;
        return false;
    }
    
    // Convert to interleaved format (x, y, z, u, v, r, g, b)
    outMesh.vertices.clear();
    for (const auto& v : vertices) {
        outMesh.vertices.push_back(v.x);
        outMesh.vertices.push_back(v.y);
        outMesh.vertices.push_back(v.z);
        outMesh.vertices.push_back(v.u);
        outMesh.vertices.push_back(v.v);
        outMesh.vertices.push_back(v.r);
        outMesh.vertices.push_back(v.g);
        outMesh.vertices.push_back(v.b);
    }
    
    outMesh.indices = indices;
    
    std::cout << "[OBJLoader] Successfully loaded OBJ: " << filepath << std::endl;
    std::cout << "[OBJLoader]   Vertices: " << vertices.size() << std::endl;
    std::cout << "[OBJLoader]   Triangles: " << indices.size() / 3 << std::endl;
    std::cout << "[OBJLoader]   Has UVs: " << (outMesh.hasTexCoords ? "Yes" : "No") << std::endl;
    std::cout << "[OBJLoader]   Has Vertex Colors: " << (outMesh.hasVertexColors ? "Yes" : "No") << std::endl;
    std::cout << "[OBJLoader]   Materials: " << outMesh.materials.size() << std::endl;
    
    return true;
}
