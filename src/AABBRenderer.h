#pragma once

#include <glad/glad.h>
#include "vendor/glm/glm.hpp"
#include "AABB.h"
#include "Shader.h"

/**
 * AABBRenderer - Renders AABB wireframes for debug visualization
 * 
 * This class provides functionality to render the edges of AABBs
 * using OpenGL line drawing for collision debugging purposes.
 */
class AABBRenderer
{
public:
    AABBRenderer();
    ~AABBRenderer();

    /**
     * Initializes the OpenGL resources for AABB rendering
     * Must be called after OpenGL context is initialized
     */
    void Initialize();

    /**
     * Renders an AABB as wireframe lines
     * 
     * @param aabb The AABB to render
     * @param shader The shader to use for rendering
     * @param view The view matrix
     * @param projection The projection matrix
     * @param color The color of the lines (default: pink)
     */
    void Draw(const AABB& aabb, Shader& shader, 
              const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& color = glm::vec3(1.0f, 0.4f, 0.7f));

    /**
     * Cleans up OpenGL resources
     */
    void Cleanup();

    /**
     * Checks if the renderer is initialized
     */
    bool IsInitialized() const { return m_initialized; }

private:
    GLuint m_vao;
    GLuint m_vbo;
    bool m_initialized;
};
