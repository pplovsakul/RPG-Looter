#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Helper function to parse a face vertex (handles v, v/vt, v/vt/vn, v//vn formats)
static unsigned int ParseFaceVertex(const std::string& vertexStr) {
    // Extract just the vertex index (before first slash)
    size_t slashPos = vertexStr.find('/');
    if (slashPos != std::string::npos) {
        return std::stoi(vertexStr.substr(0, slashPos));
    }
    return std::stoi(vertexStr);
}

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
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
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
            // Face - can be triangle or quad
            // Support formats: v, v/vt, v/vt/vn, v//vn
            // Note: OBJ indices are 1-based, we convert to 0-based
            std::vector<unsigned int> faceIndices;
            std::string vertexStr;
            
            while (iss >> vertexStr) {
                unsigned int vIndex = ParseFaceVertex(vertexStr);
                faceIndices.push_back(vIndex - 1); // Convert to 0-based
            }
            
            // Triangulate if needed
            if (faceIndices.size() == 3) {
                // Triangle - add directly
                tempIndices.push_back(faceIndices[0]);
                tempIndices.push_back(faceIndices[1]);
                tempIndices.push_back(faceIndices[2]);
            }
            else if (faceIndices.size() == 4) {
                // Quad - split into two triangles
                tempIndices.push_back(faceIndices[0]);
                tempIndices.push_back(faceIndices[1]);
                tempIndices.push_back(faceIndices[2]);
                
                tempIndices.push_back(faceIndices[0]);
                tempIndices.push_back(faceIndices[2]);
                tempIndices.push_back(faceIndices[3]);
            }
            else if (faceIndices.size() > 4) {
                // Polygon - triangulate using fan method
                for (size_t i = 1; i < faceIndices.size() - 1; ++i) {
                    tempIndices.push_back(faceIndices[0]);
                    tempIndices.push_back(faceIndices[i]);
                    tempIndices.push_back(faceIndices[i + 1]);
                }
            }
        }
        // Ignore other lines (mtllib, usemtl, o, g, s, vt, vn, etc.)
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
