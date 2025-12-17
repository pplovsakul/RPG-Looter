#include "Texture.h"
#include "Debug.h"
#include <glad/glad.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb/stb_image.h"

Texture::Texture(const std::string& path)
    : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr),
      m_Width(0), m_Height(0), m_BPP(0) {
    
    // Flip texture vertically (OpenGL expects texture origin at bottom-left)
    stbi_set_flip_vertically_on_load(1);
    
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
    
    if (!m_LocalBuffer) {
        std::cerr << "[Texture] Failed to load texture: " << path << std::endl;
        std::cerr << "[Texture] stbi error: " << stbi_failure_reason() << std::endl;
        return;
    }
    
    std::cout << "[Texture] Successfully loaded: " << path << std::endl;
    std::cout << "[Texture]   Size: " << m_Width << "x" << m_Height << ", Channels: " << m_BPP << std::endl;
    
    GLCall(glGenTextures(1, &m_RendererID));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
    
    // Set texture parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    
    // Upload texture data to GPU
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height,
                        0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    
    // Free local buffer after uploading to GPU
    if (m_LocalBuffer) {
        stbi_image_free(m_LocalBuffer);
        m_LocalBuffer = nullptr;
    }
}

Texture::~Texture() {
    if (m_RendererID != 0) {
        GLCall(glDeleteTextures(1, &m_RendererID));
    }
}

void Texture::Bind(unsigned int slot) const {
    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const {
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
