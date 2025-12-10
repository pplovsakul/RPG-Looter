#pragma once

#include <glad/glad.h>
#include <iostream>
#include <assert.h>

#define ASSERT(x) if (!(x)) assert(false)

inline void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

inline bool GLCheckError() {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << ")" << std::endl;
        return false;
    }
    return true;
}

#ifdef DEBUG
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLCheckError())
#else
#define GLCall(x) x
#endif
