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
    
    // Only create OpenGL texture if we have a valid GL context
    // Check if OpenGL is initialized by testing if we can get a function pointer
    if (glGenTextures != nullptr) {
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
    } else {
        // No OpenGL context - keep the image data in memory
        // This allows testing without a full GL context
        std::cout << "[Texture] No OpenGL context - image data loaded but not uploaded to GPU" << std::endl;
    }
}

Texture::~Texture() {
    if (m_RendererID != 0 && glDeleteTextures != nullptr) {
        GLCall(glDeleteTextures(1, &m_RendererID));
    }
    if (m_LocalBuffer) {
        stbi_image_free(m_LocalBuffer);
    }
}

void Texture::Bind(unsigned int slot) const {
    if (m_RendererID != 0 && glActiveTexture != nullptr) {
        GLCall(glActiveTexture(GL_TEXTURE0 + slot));
        GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
    }
}

void Texture::Unbind() const {
    if (glBindTexture != nullptr) {
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    }
}
