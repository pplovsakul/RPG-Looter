#include "Mesh.h"

unsigned int Mesh::AddVertex(const Vertex& vertex) {
    m_Vertices.push_back(vertex);
    return static_cast<unsigned int>(m_Vertices.size() - 1);
}

void Mesh::AddFace(const Face& face) {
    m_Faces.push_back(face);
}

void Mesh::AddMaterial(const Material& material) {
    m_Materials[material.name] = material;
}

const Material* Mesh::GetMaterial(const std::string& name) const {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) {
        return &it->second;
    }
    return nullptr;
}

void Mesh::Clear() {
    m_Vertices.clear();
    m_Faces.clear();
    m_Materials.clear();
    m_Name.clear();
}

std::vector<float> Mesh::GetInterleavedVertexData() const {
    std::vector<float> data;
    data.reserve(m_Vertices.size() * 8); // 3 pos + 3 normal + 2 texcoord = 8 floats per vertex

    for (const auto& vertex : m_Vertices) {
        // Position
        data.push_back(vertex.position[0]);
        data.push_back(vertex.position[1]);
        data.push_back(vertex.position[2]);
        
        // Normal
        data.push_back(vertex.normal[0]);
        data.push_back(vertex.normal[1]);
        data.push_back(vertex.normal[2]);
        
        // Texture coordinates
        data.push_back(vertex.texCoords[0]);
        data.push_back(vertex.texCoords[1]);
    }

    return data;
}

std::vector<unsigned int> Mesh::GetIndexData() const {
    std::vector<unsigned int> indices;
    indices.reserve(m_Faces.size() * 3); // 3 indices per triangle

    for (const auto& face : m_Faces) {
        indices.push_back(face.indices[0]);
        indices.push_back(face.indices[1]);
        indices.push_back(face.indices[2]);
    }

    return indices;
}
