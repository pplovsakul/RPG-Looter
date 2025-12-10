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
        else if (prefix == "Ka") {
            // Ambient color
            iss >> currentMaterial.ambientColor.r 
                >> currentMaterial.ambientColor.g 
                >> currentMaterial.ambientColor.b;
        }
        else if (prefix == "Kd") {
            // Diffuse color
            iss >> currentMaterial.diffuseColor.r 
                >> currentMaterial.diffuseColor.g 
                >> currentMaterial.diffuseColor.b;
        }
        else if (prefix == "Ks") {
            // Specular color
            iss >> currentMaterial.specularColor.r 
                >> currentMaterial.specularColor.g 
                >> currentMaterial.specularColor.b;
        }
        else if (prefix == "Ns") {
            // Shininess
            iss >> currentMaterial.shininess;
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
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<Material> materials;
    
    auto model = std::make_unique<ModelComponent>();
    ModelComponent::Mesh currentMesh;
    std::string currentMaterialName;
    
    // Map to deduplicate vertices
    struct VertexKey {
        int posIdx, uvIdx, normIdx;
        bool operator==(const VertexKey& other) const {
            return posIdx == other.posIdx && uvIdx == other.uvIdx && normIdx == other.normIdx;
        }
    };
    struct VertexKeyHash {
        size_t operator()(const VertexKey& k) const {
            return ((size_t)k.posIdx * 100000) + ((size_t)k.uvIdx * 1000) + (size_t)k.normIdx;
        }
    };
    std::unordered_map<VertexKey, unsigned int, VertexKeyHash> vertexCache;

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
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
        else if (prefix == "vn") {
            // Vertex normal
            glm::vec3 norm;
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(glm::normalize(norm));
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
                model->meshes.push_back(currentMesh);
                currentMesh = ModelComponent::Mesh();
                vertexCache.clear();
            }
            iss >> currentMaterialName;
            currentMesh.materialName = currentMaterialName;
        }
        else if (prefix == "f") {
            // Face - can be triangle or quad
            std::vector<std::string> vertexData;
            std::string vd;
            while (iss >> vd) {
                vertexData.push_back(vd);
            }

            // Triangulate if necessary (quads -> 2 triangles)
            for (size_t i = 0; i < vertexData.size() - 2; i++) {
                std::string verts[3] = {
                    vertexData[0],
                    vertexData[i + 1],
                    vertexData[i + 2]
                };

                for (int j = 0; j < 3; j++) {
                    // Parse vertex data (format: v/vt/vn or v/vt or v//vn or v)
                    VertexKey key{0, 0, 0};
                    std::string& vstr = verts[j];
                    std::replace(vstr.begin(), vstr.end(), '/', ' ');
                    std::istringstream viss(vstr);
                    viss >> key.posIdx;
                    if (!viss.eof()) viss >> key.uvIdx;
                    if (!viss.eof()) viss >> key.normIdx;

                    // Check cache
                    auto it = vertexCache.find(key);
                    if (it != vertexCache.end()) {
                        currentMesh.indices.push_back(it->second);
                        continue;
                    }

                    // Add new vertex
                    if (key.posIdx > 0 && key.posIdx <= (int)positions.size()) {
                        currentMesh.vertices.push_back(positions[key.posIdx - 1]);
                    } else {
                        currentMesh.vertices.push_back(glm::vec3(0.0f));
                    }

                    if (key.normIdx > 0 && key.normIdx <= (int)normals.size()) {
                        currentMesh.normals.push_back(normals[key.normIdx - 1]);
                    } else {
                        currentMesh.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                    }

                    if (key.uvIdx > 0 && key.uvIdx <= (int)uvs.size()) {
                        currentMesh.uvs.push_back(uvs[key.uvIdx - 1]);
                    } else {
                        currentMesh.uvs.push_back(glm::vec2(0.0f));
                    }

                    unsigned int index = (unsigned int)currentMesh.vertices.size() - 1;
                    currentMesh.indices.push_back(index);
                    vertexCache[key] = index;
                }
            }
        }
    }

    // Add final mesh
    if (!currentMesh.vertices.empty()) {
        model->meshes.push_back(currentMesh);
    }

    // Apply material properties to meshes
    std::unordered_map<std::string, Material> materialMap;
    for (const auto& mat : materials) {
        materialMap[mat.name] = mat;
    }

    for (auto& mesh : model->meshes) {
        auto matIt = materialMap.find(mesh.materialName);
        if (matIt != materialMap.end()) {
            mesh.color = matIt->second.diffuseColor;
            mesh.textureName = matIt->second.diffuseTexture;
        }
    }

    std::cout << "[OBJLoader] Loaded " << model->meshes.size() << " meshes from " << objPath 
              << " (total vertices: ";
    int totalVerts = 0;
    for (const auto& m : model->meshes) totalVerts += m.vertices.size();
    std::cout << totalVerts << ")\n";
    
    return model;
}
