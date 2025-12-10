#include "Texture.h"

#include "vendor/stb_image/stb_image.h"
#include "Debug.h"
#include <algorithm>
#include <iostream>

static std::string getFileExtensionLower(const std::string& path) {
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos) return std::string();
    std::string ext = path.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
    return ext;
}

Texture::Texture(const std::string& path)
    : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{

    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
    if (!m_LocalBuffer) {
        std::cerr << "[Texture] Failed to load image: " << path << " (" << stbi_failure_reason() << ")\n";
        return;
    }

    GLCall(glGenTextures(1, &m_RendererID));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID)); // Bind without slot selection

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
    Unbind();

    if (m_LocalBuffer)
        stbi_image_free(m_LocalBuffer);
};

Texture::~Texture()
{
    GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int slot) const
{
    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}