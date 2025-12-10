#pragma once

#include "Mesh3D.h"
#include <string>
#include <memory>
#include <vector>

class OBJLoader {
public:
    // Load a single mesh from OBJ file
    static std::unique_ptr<Mesh3D> loadOBJ(const std::string& filepath);
    
    // Load multiple meshes (for complex models)
    static std::vector<std::unique_ptr<Mesh3D>> loadOBJMulti(const std::string& filepath);

private:
    static bool parseOBJFile(const std::string& filepath,
                            std::vector<glm::vec3>& positions,
                            std::vector<glm::vec3>& normals,
                            std::vector<glm::vec2>& uvs,
                            std::vector<Vertex3D>& outVertices,
                            std::vector<unsigned int>& outIndices);
    
    static void calculateNormals(std::vector<Vertex3D>& vertices,
                                const std::vector<unsigned int>& indices);
};
