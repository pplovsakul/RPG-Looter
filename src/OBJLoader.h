#pragma once

#include <vector>
#include <string>

class OBJLoader {
public:
    // Simple structure to hold mesh data
    struct MeshData {
        std::vector<float> vertices;        // Interleaved vertex data (x, y, z)
        std::vector<unsigned int> indices;  // Triangle indices
    };

    // Load an OBJ file and return mesh data
    // Returns true on success, false on failure
    static bool LoadOBJ(const std::string& filepath, MeshData& outMesh);
    
    // Get index data from mesh
    static std::vector<unsigned int> GetIndexData(const MeshData& mesh) {
        return mesh.indices;
    }
    
    // Get interleaved vertex data from mesh
    static std::vector<float> GetInterleavedVertexData(const MeshData& mesh) {
        return mesh.vertices;
    }
};
