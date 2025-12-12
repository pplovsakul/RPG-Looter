#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Debug.h"

// Function pointers for OpenGL 4.3+ compute shader functions
// These may not be available in all GLAD versions
typedef void (*PFNGLDISPATCHCOMPUTEPROC)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (*PFNGLBINDIMAGETEXTUREPROC)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (*PFNGLMEMORYBARRIERPROC)(GLbitfield barriers);

// Global function pointers
static PFNGLDISPATCHCOMPUTEPROC glDispatchCompute_ptr = nullptr;
static PFNGLBINDIMAGETEXTUREPROC glBindImageTexture_ptr = nullptr;
static PFNGLMEMORYBARRIERPROC glMemoryBarrier_ptr = nullptr;

// GL constants that might not be defined
#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER 0x91B9
#endif
#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif
#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif

// ComputeShader: Verwaltet OpenGL Compute Shader
// Compute Shaders ermöglichen parallele GPU-Berechnungen
class ComputeShader {
private:
    GLuint m_RendererID = 0;
    bool m_Available = false;

    static bool loadComputeShaderFunctions() {
        static bool loaded = false;
        static bool available = false;
        
        if (loaded) return available;
        loaded = true;
        
        // Try to load OpenGL 4.3 functions
        glDispatchCompute_ptr = (PFNGLDISPATCHCOMPUTEPROC)glfwGetProcAddress("glDispatchCompute");
        glBindImageTexture_ptr = (PFNGLBINDIMAGETEXTUREPROC)glfwGetProcAddress("glBindImageTexture");
        glMemoryBarrier_ptr = (PFNGLMEMORYBARRIERPROC)glfwGetProcAddress("glMemoryBarrier");
        
        available = (glDispatchCompute_ptr && glBindImageTexture_ptr && glMemoryBarrier_ptr);
        
        if (!available) {
            std::cerr << "OpenGL 4.3 compute shader functions not available" << std::endl;
        }
        
        return available;
    }

public:
    ComputeShader(const std::string& filepath) {
        if (!loadComputeShaderFunctions()) {
            m_Available = false;
            return;
        }
        
        std::string source = readFile(filepath);
        if (source.empty()) {
            m_Available = false;
            return;
        }
        
        m_RendererID = createShader(source);
        m_Available = (m_RendererID != 0);
    }

    ~ComputeShader() {
        if (m_RendererID) {
            GLCall(glDeleteProgram(m_RendererID));
        }
    }

    bool isAvailable() const { return m_Available; }

    void Bind() const {
        if (m_Available) {
            GLCall(glUseProgram(m_RendererID));
        }
    }

    void Unbind() const {
        GLCall(glUseProgram(0));
    }

    GLuint GetRendererID() const { return m_RendererID; }

    // Dispatch compute shader
    void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const {
        if (m_Available && glDispatchCompute_ptr) {
            glDispatchCompute_ptr(numGroupsX, numGroupsY, numGroupsZ);
        }
    }

    // Warte bis Compute Shader fertig ist
    void Wait() const {
        if (m_Available && glMemoryBarrier_ptr) {
            glMemoryBarrier_ptr(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }

    // Setze Uniform-Variablen
    void SetUniform1i(const std::string& name, int value) {
        GLCall(glUniform1i(GetUniformLocation(name), value));
    }

    void SetUniform1f(const std::string& name, float value) {
        GLCall(glUniform1f(GetUniformLocation(name), value));
    }

    void SetUniform3f(const std::string& name, float v0, float v1, float v2) {
        GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
    }

    void SetUniformVec3(const std::string& name, const glm::vec3& vec) {
        GLCall(glUniform3f(GetUniformLocation(name), vec.x, vec.y, vec.z));
    }

private:
    std::string readFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open compute shader file: " << filepath << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    GLuint createShader(const std::string& source) {
        GLuint program = glCreateProgram();
        GLuint shader = compileShader(GL_COMPUTE_SHADER, source);
        
        if (shader == 0) {
            glDeleteProgram(program);
            return 0;
        }
        
        GLCall(glAttachShader(program, shader));
        GLCall(glLinkProgram(program));
        GLCall(glValidateProgram(program));

        // Check for linking errors
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length * sizeof(char));
            glGetProgramInfoLog(program, length, &length, message);
            std::cerr << "Failed to link compute shader program!" << std::endl;
            std::cerr << message << std::endl;
            glDeleteProgram(program);
            glDeleteShader(shader);
            return 0;
        }

        GLCall(glDeleteShader(shader));
        return program;
    }

    GLuint compileShader(GLenum type, const std::string& source) {
        GLuint id = glCreateShader(type);
        const char* src = source.c_str();
        GLCall(glShaderSource(id, 1, &src, nullptr));
        GLCall(glCompileShader(id));

        // Error handling
        int result;
        GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
        if (result == GL_FALSE) {
            int length;
            GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
            char* message = (char*)alloca(length * sizeof(char));
            GLCall(glGetShaderInfoLog(id, length, &length, message));
            std::cerr << "Failed to compile compute shader!" << std::endl;
            std::cerr << message << std::endl;
            GLCall(glDeleteShader(id));
            return 0;
        }

        return id;
    }

    GLint GetUniformLocation(const std::string& name) const {
        GLCall(GLint location = glGetUniformLocation(m_RendererID, name.c_str()));
        // Note: Uniform location warnings disabled to avoid spam for optional uniforms
        return location;
    }
};
