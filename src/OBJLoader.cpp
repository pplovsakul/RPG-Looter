#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

std::unique_ptr<Mesh3D> OBJLoader::loadOBJ(const std::string& filepath) {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    
    if (!parseOBJFile(filepath, positions, normals, uvs, vertices, indices)) {
        std::cerr << "Failed to load OBJ file: " << filepath << std::endl;
        return nullptr;
    }
    
    // Calculate normals if not provided
    if (normals.empty()) {
        calculateNormals(vertices, indices);
    }
    
    return std::make_unique<Mesh3D>(vertices, indices);
}

std::vector<std::unique_ptr<Mesh3D>> OBJLoader::loadOBJMulti(const std::string& filepath) {
    std::vector<std::unique_ptr<Mesh3D>> meshes;
    
    // For simplicity, just load as single mesh for now
    auto mesh = loadOBJ(filepath);
    if (mesh) {
        meshes.push_back(std::move(mesh));
    }
    
    return meshes;
}

bool OBJLoader::parseOBJFile(const std::string& filepath,
                             std::vector<glm::vec3>& positions,
                             std::vector<glm::vec3>& normals,
                             std::vector<glm::vec2>& uvs,
                             std::vector<Vertex3D>& outVertices,
                             std::vector<unsigned int>& outIndices) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open OBJ file: " << filepath << std::endl;
        return false;
    }
    
    std::string line;
    std::unordered_map<std::string, unsigned int> vertexCache;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            // Vertex position
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vn") {
            // Vertex normal
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (prefix == "vt") {
            // Texture coordinate
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (prefix == "f") {
            // Face
            std::vector<std::string> faceVertices;
            std::string vertexStr;
            
            while (iss >> vertexStr) {
                faceVertices.push_back(vertexStr);
            }
            
            // Process face (triangulate if needed)
            for (size_t i = 0; i < faceVertices.size(); i++) {
                const std::string& vertStr = faceVertices[i];
                
                // Check cache
                auto it = vertexCache.find(vertStr);
                if (it != vertexCache.end()) {
                    outIndices.push_back(it->second);
                    continue;
                }
                
                // Parse vertex string (format: v/vt/vn or v//vn or v/vt or v)
                std::istringstream vss(vertStr);
                std::string indexStr;
                std::vector<int> vertexIndices;
                
                while (std::getline(vss, indexStr, '/')) {
                    if (!indexStr.empty()) {
                        vertexIndices.push_back(std::stoi(indexStr) - 1); // OBJ indices are 1-based
                    } else {
                        vertexIndices.push_back(-1);
                    }
                }
                
                Vertex3D vertex;
                
                // Position
                if (vertexIndices.size() > 0 && vertexIndices[0] >= 0 && vertexIndices[0] < positions.size()) {
                    vertex.position = positions[vertexIndices[0]];
                }
                
                // Texture coordinate
                if (vertexIndices.size() > 1 && vertexIndices[1] >= 0 && vertexIndices[1] < uvs.size()) {
                    vertex.texCoords = uvs[vertexIndices[1]];
                }
                
                // Normal
                if (vertexIndices.size() > 2 && vertexIndices[2] >= 0 && vertexIndices[2] < normals.size()) {
                    vertex.normal = normals[vertexIndices[2]];
                }
                
                // Default color
                vertex.color = glm::vec3(1.0f);
                
                unsigned int newIndex = outVertices.size();
                outVertices.push_back(vertex);
                outIndices.push_back(newIndex);
                vertexCache[vertStr] = newIndex;
            }
            
            // If face has more than 3 vertices, triangulate (simple fan triangulation)
            if (faceVertices.size() > 3) {
                // Already added all vertices, now add extra indices for triangulation
                size_t startIdx = outIndices.size() - faceVertices.size();
                for (size_t i = 3; i < faceVertices.size(); i++) {
                    outIndices.push_back(outIndices[startIdx]);
                    outIndices.push_back(outIndices[startIdx + i - 1]);
                    outIndices.push_back(outIndices[startIdx + i]);
                }
            }
        }
    }
    
    file.close();
    return true;
}

void OBJLoader::calculateNormals(std::vector<Vertex3D>& vertices,
                                const std::vector<unsigned int>& indices) {
    // Initialize all normals to zero
    for (auto& vertex : vertices) {
        vertex.normal = glm::vec3(0.0f);
    }
    
    // Calculate face normals and accumulate
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];
        
        glm::vec3 v0 = vertices[i0].position;
        glm::vec3 v1 = vertices[i1].position;
        glm::vec3 v2 = vertices[i2].position;
        
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }
    
    // Normalize all normals
    for (auto& vertex : vertices) {
        if (glm::length(vertex.normal) > 0.0f) {
            vertex.normal = glm::normalize(vertex.normal);
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default up
        }
    }
}
