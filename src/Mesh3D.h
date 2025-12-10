#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "vendor/glm/glm.hpp"
#include <vector>
#include <memory>

struct Vertex3D {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color;

    Vertex3D() 
        : position(0.0f), normal(0.0f, 1.0f, 0.0f), texCoords(0.0f), color(1.0f) {}
    
    Vertex3D(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& uv, const glm::vec3& col = glm::vec3(1.0f))
        : position(pos), normal(norm), texCoords(uv), color(col) {}
};

class Mesh3D {
private:
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<VertexBuffer> vbo;
    std::unique_ptr<IndexBuffer> ibo;
    
    unsigned int materialID;
    bool buffersSetup;

public:
    Mesh3D(const std::vector<Vertex3D>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh3D() = default;
    
    void setupBuffers();
    void draw() const;
    void updateVertices(const std::vector<Vertex3D>& newVertices);
    
    // Getters
    const std::vector<Vertex3D>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }
    size_t getVertexCount() const { return vertices.size(); }
    size_t getIndexCount() const { return indices.size(); }
    
    void setMaterialID(unsigned int id) { materialID = id; }
    unsigned int getMaterialID() const { return materialID; }
};
