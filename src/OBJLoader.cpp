#include "OBJLoader.h"
#include "Components.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>

std::vector<OBJLoader::Material> OBJLoader::loadMTL(const std::string& mtlPath) {
    std::vector<Material> materials;
    std::ifstream file(mtlPath);
    if (!file.is_open()) {
        std::cerr << "[OBJLoader] Could not open MTL file: " << mtlPath << "\n";
        return materials;
    }

    Material currentMaterial;
    bool hasMaterial = false;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "newmtl") {
            if (hasMaterial) {
                materials.push_back(currentMaterial);
            }
            currentMaterial = Material();
            iss >> currentMaterial.name;
            hasMaterial = true;
        }
        else if (prefix == "Kd") {
            // Diffuse color
            iss >> currentMaterial.diffuseColor.r 
                >> currentMaterial.diffuseColor.g 
                >> currentMaterial.diffuseColor.b;
        }
        else if (prefix == "map_Kd") {
            // Diffuse texture
            std::string texPath;
            iss >> texPath;
            // Extract just the filename without extension
            size_t lastSlash = texPath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                texPath = texPath.substr(lastSlash + 1);
            }
            size_t lastDot = texPath.find_last_of(".");
            if (lastDot != std::string::npos) {
                texPath = texPath.substr(0, lastDot);
            }
            currentMaterial.diffuseTexture = texPath;
        }
        else if (prefix == "d" || prefix == "Tr") {
            // Opacity
            iss >> currentMaterial.opacity;
        }
    }

    if (hasMaterial) {
        materials.push_back(currentMaterial);
    }

    std::cout << "[OBJLoader] Loaded " << materials.size() << " materials from " << mtlPath << "\n";
    return materials;
}

std::unique_ptr<ModelComponent> OBJLoader::loadOBJ(const std::string& objPath) {
    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "[OBJLoader] Could not open OBJ file: " << objPath << "\n";
        return nullptr;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    
    Mesh currentMesh;
    std::string currentMaterialName;
    std::unordered_map<std::string, int> vertexCache;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "mtllib") {
            // Load material library
            std::string mtlFile;
            iss >> mtlFile;
            // Construct full path
            std::string dir = objPath.substr(0, objPath.find_last_of("/\\") + 1);
            materials = loadMTL(dir + mtlFile);
        }
        else if (prefix == "v") {
            // Vertex position
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vt") {
            // Texture coordinate
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (prefix == "usemtl") {
            // Start new mesh with this material
            if (!currentMesh.vertices.empty()) {
                meshes.push_back(currentMesh);
                currentMesh = Mesh();
                vertexCache.clear();
            }
            iss >> currentMaterialName;
            currentMesh.materialName = currentMaterialName;
        }
        else if (prefix == "f") {
            // Face (triangle)
            std::string vertexData[3];
            iss >> vertexData[0] >> vertexData[1] >> vertexData[2];

            for (int i = 0; i < 3; i++) {
                // Check cache
                auto it = vertexCache.find(vertexData[i]);
                if (it != vertexCache.end()) {
                    currentMesh.indices.push_back(it->second);
                    continue;
                }

                // Parse vertex data (format: v/vt/vn or v/vt or v)
                int vIdx = 0, vtIdx = 0, vnIdx = 0;
                std::replace(vertexData[i].begin(), vertexData[i].end(), '/', ' ');
                std::istringstream viss(vertexData[i]);
                viss >> vIdx;
                if (!viss.eof()) viss >> vtIdx;
                if (!viss.eof()) viss >> vnIdx;

                // OBJ indices are 1-based
                if (vIdx > 0 && vIdx <= (int)positions.size()) {
                    currentMesh.vertices.push_back(positions[vIdx - 1]);
                }
                if (vtIdx > 0 && vtIdx <= (int)uvs.size()) {
                    currentMesh.uvs.push_back(uvs[vtIdx - 1]);
                } else {
                    currentMesh.uvs.push_back(glm::vec2(0.0f));
                }

                int index = (int)currentMesh.vertices.size() - 1;
                currentMesh.indices.push_back(index);
                vertexCache[vertexData[i]] = index;
            }
        }
    }

    if (!currentMesh.vertices.empty()) {
        meshes.push_back(currentMesh);
    }

    std::cout << "[OBJLoader] Loaded " << meshes.size() << " meshes from " << objPath << "\n";
    
    return convertToModelComponent(meshes, materials);
}

std::unique_ptr<ModelComponent> OBJLoader::convertToModelComponent(
    const std::vector<Mesh>& meshes,
    const std::vector<Material>& materials
) {
    auto model = std::make_unique<ModelComponent>();

    // Create a material lookup map
    std::unordered_map<std::string, Material> materialMap;
    for (const auto& mat : materials) {
        materialMap[mat.name] = mat;
    }

    int layer = 0;
    for (const auto& mesh : meshes) {
        if (mesh.vertices.empty()) continue;

        // Calculate bounding box for the mesh in 2D (using X and Y)
        glm::vec2 minPos(FLT_MAX);
        glm::vec2 maxPos(-FLT_MAX);
        
        for (const auto& v : mesh.vertices) {
            minPos.x = std::min(minPos.x, v.x);
            minPos.y = std::min(minPos.y, v.y);
            maxPos.x = std::max(maxPos.x, v.x);
            maxPos.y = std::max(maxPos.y, v.y);
        }

        glm::vec2 center = (minPos + maxPos) * 0.5f;
        glm::vec2 size = maxPos - minPos;

        // Create a shape for this mesh
        ModelComponent::Shape shape;
        
        // Determine shape type based on vertex count
        if (mesh.vertices.size() == 3) {
            shape.type = ModelComponent::ShapeType::Triangle;
        } else {
            // Default to textured quad for meshes with 4+ vertices
            shape.type = ModelComponent::ShapeType::TexturedQuad;
        }

        shape.position = center * 100.0f; // Scale up for screen coordinates
        shape.rotation = 0.0f;
        shape.size = size * 100.0f; // Scale up for screen coordinates
        shape.scale = glm::vec2(1.0f);
        
        // Apply material properties
        auto matIt = materialMap.find(mesh.materialName);
        if (matIt != materialMap.end()) {
            shape.color = matIt->second.diffuseColor;
            shape.textureName = matIt->second.diffuseTexture;
        } else {
            shape.color = glm::vec3(1.0f);
            shape.textureName = "";
        }

        shape.filled = true;
        shape.layer = layer++;

        model->shapes.push_back(shape);
    }

    std::cout << "[OBJLoader] Created ModelComponent with " << model->shapes.size() << " shapes\n";
    return model;
}
