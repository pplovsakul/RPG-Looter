#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Debug.h"

// ComputeShader: Verwaltet OpenGL Compute Shader
// Compute Shaders ermöglichen parallele GPU-Berechnungen
class ComputeShader {
private:
    GLuint m_RendererID = 0;

public:
    ComputeShader(const std::string& filepath) {
        std::string source = readFile(filepath);
        m_RendererID = createShader(source);
    }

    ~ComputeShader() {
        if (m_RendererID) {
            GLCall(glDeleteProgram(m_RendererID));
        }
    }

    void Bind() const {
        GLCall(glUseProgram(m_RendererID));
    }

    void Unbind() const {
        GLCall(glUseProgram(0));
    }

    GLuint GetRendererID() const { return m_RendererID; }

    // Dispatch compute shader
    void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const {
        GLCall(glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
    }

    // Warte bis Compute Shader fertig ist
    void Wait() const {
        GLCall(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
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
        if (location == -1) {
            std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
        }
        return location;
    }
};
