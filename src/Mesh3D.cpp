#include "Mesh3D.h"
#include <glad/glad.h>

Mesh3D::Mesh3D(const std::vector<Vertex3D>& verts, const std::vector<unsigned int>& inds)
    : vertices(verts), indices(inds), materialID(0), buffersSetup(false) {
}

void Mesh3D::setupBuffers() {
    if (buffersSetup) return;
    
    // Create VAO
    vao = std::make_unique<VertexArray>();
    vao->Bind();
    
    // Create VBO with vertex data
    vbo = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex3D));
    
    // Setup vertex attribute layout
    VertexBufferLayout layout;
    layout.AddFloat(3); // position
    layout.AddFloat(3); // normal
    layout.AddFloat(2); // texCoords
    layout.AddFloat(3); // color
    
    vao->AddBuffer(*vbo, layout);
    
    // Create IBO with index data
    ibo = std::make_unique<IndexBuffer>(indices.data(), indices.size());
    
    vao->Unbind();
    vbo->Unbind();
    ibo->Unbind();
    
    buffersSetup = true;
}

void Mesh3D::draw() const {
    if (!buffersSetup) return;
    
    vao->Bind();
    ibo->Bind();
    
    glDrawElements(GL_TRIANGLES, ibo->GetCount(), GL_UNSIGNED_INT, nullptr);
    
    vao->Unbind();
    ibo->Unbind();
}

void Mesh3D::updateVertices(const std::vector<Vertex3D>& newVertices) {
    vertices = newVertices;
    
    if (buffersSetup && vbo) {
        vbo->Bind();
        vbo->UpdateData(vertices.data(), vertices.size() * sizeof(Vertex3D));
        vbo->Unbind();
    }
}
