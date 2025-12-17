#pragma once

#include <vector>
#include <string>
#include <map>
#include "Material.h"

class OBJLoader {
public:
    // Structure to hold mesh data with texture coordinates
    struct MeshData {
        std::vector<float> vertices;        // Interleaved vertex data (x, y, z, u, v)
        std::vector<unsigned int> indices;  // Triangle indices
        std::map<std::string, Material> materials; // Materials by name
        std::string activeMaterial;         // Currently active material name
        bool hasTexCoords = false;          // Whether mesh has texture coordinates
    };

    // Load an OBJ file and return mesh data with MTL support
    // Returns true on success, false on failure
    static bool LoadOBJ(const std::string& filepath, MeshData& outMesh);
    
    // Get index data from mesh
    static std::vector<unsigned int> GetIndexData(const MeshData& mesh) {
        return mesh.indices;
    }
    
    // Get interleaved vertex data from mesh (x, y, z, u, v)
    static std::vector<float> GetInterleavedVertexData(const MeshData& mesh) {
        return mesh.vertices;
    }
};
