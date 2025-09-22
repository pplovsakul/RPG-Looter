#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

#include <memory>
#include <string>
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"


//Declarations
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class Texture;
class VertexBufferLayout;

struct RenderObject {
    std::unique_ptr<VertexArray> va;
    std::unique_ptr<VertexBuffer> vb;
    std::unique_ptr<IndexBuffer> ib;
    glm::vec4 color{ 1.0f };
    std::unique_ptr<Texture> texture;
	VertexBufferLayout layout;

    std::function<glm::vec2()> getPosition;

    glm::vec2 position{ 0.0f, 0.0f };
    float rotationDeg = 0.0f;
    glm::vec2 scale{ 1.0f, 1.0f };
    glm::mat4 model{ 1.0f };

    bool needsUpdate = true;

    RenderObject() = default;

    // Neuer Constructor: nur Vertices + Indices + Layout
    RenderObject(const void* vertices, size_t vertexSize,
        const std::vector<unsigned int>& indices,
        const glm::vec2& pos = glm::vec2(0.0f),
        std::unique_ptr<Texture> texture = nullptr);

    void UpdateTexture(std::unique_ptr<Texture> newTex);

    void UpdateVertices(const void* vertices, size_t vertexSize);
    void UpdateIndices(const std::vector<unsigned int>& indices);
    
    void UpdateColor(const glm::vec4& newColor);

	void UpdateModelMatrix(const glm::vec2& pos);
};
class Renderer {
public:
    void Clear() const;

    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void Draw_RenderObject(RenderObject& obj,  Shader& shader, const glm::mat4& proj, const glm::vec2& pos) const;

    void Draw_TrianglesFan(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
};