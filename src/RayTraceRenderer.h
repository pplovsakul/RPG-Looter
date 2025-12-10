#pragma once
#include <vector>
#include "vendor/glm/glm.hpp"
#include "RayTracer.h"
#include "Debug.h"
#include <glad/glad.h>

// Uploads CPU ray-traced image to a GL texture and draws a full-screen quad
class RayTraceRenderer {
public:
    int width, height;
    GLuint texture = 0;
    GLuint vao = 0, vbo = 0;

    RayTracer tracer;

    RayTraceRenderer(int w, int h)
        : width(w), height(h), tracer(w, h) {
        // Create texture
        GLCall(glGenTextures(1, &texture));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));

        // Fullscreen quad
        float quad[16] = {
            // pos     // uv
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));
        GLCall(glGenBuffers(1, &vbo));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW));
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
        GLCall(glBindVertexArray(0));
    }

    ~RayTraceRenderer() {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (texture) glDeleteTextures(1, &texture);
    }

    void draw(GLuint shaderProgram) {
        // Run CPU render
        std::vector<uint32_t> pixels = tracer.render();

        // Upload to texture
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        GLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data()));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));

        // Use a simple shader that draws the texture
        GLCall(glUseProgram(shaderProgram));
        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        GLint loc = glGetUniformLocation(shaderProgram, "u_Texture");
        if (loc >= 0) glUniform1i(loc, 0);

        GLCall(glBindVertexArray(vao));
        GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
        GLCall(glBindVertexArray(0));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        GLCall(glUseProgram(0));
    }
};