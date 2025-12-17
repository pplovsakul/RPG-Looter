#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool OBJLoader::LoadOBJ(const std::string& filepath, MeshData& outMesh) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open OBJ file: " << filepath << std::endl;
        return false;
    }

    std::vector<float> tempVertices;
    std::vector<unsigned int> tempIndices;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            // Vertex position
            float x, y, z;
            iss >> x >> y >> z;
            tempVertices.push_back(x);
            tempVertices.push_back(y);
            tempVertices.push_back(z);
        }
        else if (prefix == "f") {
            // Face (triangle)
            // Support simple format: f v1 v2 v3
            // Note: OBJ indices are 1-based, we convert to 0-based
            unsigned int v1, v2, v3;
            char slash;
            
            // Try to parse different face formats
            // Simple format: f v1 v2 v3
            if (iss >> v1 >> v2 >> v3) {
                tempIndices.push_back(v1 - 1);
                tempIndices.push_back(v2 - 1);
                tempIndices.push_back(v3 - 1);
            }
            // If there are slashes (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3), ignore them for now
            // This simple loader only handles vertex positions
        }
        // Ignore other lines (comments, texture coords, normals, etc.)
    }
    
    file.close();
    
    if (tempVertices.empty() || tempIndices.empty()) {
        std::cerr << "ERROR: OBJ file contains no geometry: " << filepath << std::endl;
        return false;
    }
    
    outMesh.vertices = tempVertices;
    outMesh.indices = tempIndices;
    
    std::cout << "Successfully loaded OBJ: " << filepath << std::endl;
    std::cout << "  Vertices: " << tempVertices.size() / 3 << std::endl;
    std::cout << "  Triangles: " << tempIndices.size() / 3 << std::endl;
    
    return true;
}
