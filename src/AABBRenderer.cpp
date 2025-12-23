#include "AABBRenderer.h"
#include "vendor/glm/gtc/matrix_transform.hpp"

AABBRenderer::AABBRenderer()
    : m_vao(0), m_vbo(0), m_initialized(false)
{
}

AABBRenderer::~AABBRenderer()
{
    Cleanup();
}

void AABBRenderer::Initialize()
{
    if (m_initialized) return;

    // Create VAO and VBO for line rendering
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // AABB has 12 edges, each edge needs 2 vertices = 24 vertices
    // Each vertex has 3 floats (x, y, z)
    glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_initialized = true;
}

void AABBRenderer::Draw(const AABB& aabb, Shader& shader,
                        const glm::mat4& view, const glm::mat4& projection,
                        const glm::vec3& color)
{
    if (!m_initialized) return;

    // Get the 8 corners of the AABB
    // min = (minX, minY, minZ), max = (maxX, maxY, maxZ)
    glm::vec3 corners[8] = {
        glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z), // 0: min corner
        glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z), // 1
        glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z), // 2
        glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z), // 3
        glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z), // 4
        glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z), // 5
        glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z), // 6: max corner
        glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z)  // 7
    };

    // 12 edges of the AABB (each edge is defined by 2 corner indices)
    // Bottom face edges: 0-1, 1-2, 2-3, 3-0 (actually front face at minZ)
    // Top face edges: 4-5, 5-6, 6-7, 7-4 (actually back face at maxZ)
    // Connecting edges: 0-4, 1-5, 2-6, 3-7
    int edgeIndices[24] = {
        // Front face (minZ)
        0, 1,  1, 2,  2, 3,  3, 0,
        // Back face (maxZ)
        4, 5,  5, 6,  6, 7,  7, 4,
        // Connecting edges
        0, 4,  1, 5,  2, 6,  3, 7
    };

    // Build vertex data for lines
    float vertices[24 * 3]; // 24 vertices * 3 floats each
    for (int i = 0; i < 24; ++i)
    {
        vertices[i * 3]     = corners[edgeIndices[i]].x;
        vertices[i * 3 + 1] = corners[edgeIndices[i]].y;
        vertices[i * 3 + 2] = corners[edgeIndices[i]].z;
    }

    // Update VBO with new vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Set up shader
    shader.Bind();
    shader.SetUniformMat4f("u_Model", glm::mat4(1.0f)); // Identity - AABB is already in world space
    shader.SetUniformMat4f("u_View", view);
    shader.SetUniformMat4f("u_Projection", projection);
    shader.SetUniform3f("u_Color", color.r, color.g, color.b);

    // Draw lines
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}

void AABBRenderer::Cleanup()
{
    if (!m_initialized) return;

    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    m_initialized = false;
}
