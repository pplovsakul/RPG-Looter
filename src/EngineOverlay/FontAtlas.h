#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "../vendor/glm/glm.hpp"

namespace EngineUI {

struct Glyph {
    glm::vec2 uvMin;
    glm::vec2 uvMax;
    glm::vec2 size;
    glm::vec2 bearing;
    float advance;
};

/**
 * FontAtlas - Manages bitmap font texture and glyph information
 * 
 * Creates a simple ASCII bitmap font atlas
 */
class FontAtlas {
public:
    FontAtlas();
    ~FontAtlas();
    
    bool init();
    void shutdown();
    
    GLuint getTexture() const { return m_textureID; }
    
    const Glyph* getGlyph(char c) const;
    
    float getLineHeight() const { return m_lineHeight; }
    glm::vec2 measureText(const char* text) const;
    
private:
    void createBitmapFont();
    
    GLuint m_textureID = 0;
    std::unordered_map<char, Glyph> m_glyphs;
    
    float m_lineHeight = 16.0f;
    int m_atlasWidth = 512;
    int m_atlasHeight = 512;
};

} // namespace EngineUI
