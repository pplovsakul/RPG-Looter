#include "FontAtlas.h"
#include "../Debug.h"
#include <cstring>
#include <vector>

namespace EngineUI {

FontAtlas::FontAtlas() {
}

FontAtlas::~FontAtlas() {
    shutdown();
}

bool FontAtlas::init() {
    createBitmapFont();
    return m_textureID != 0;
}

void FontAtlas::shutdown() {
    if (m_textureID) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    m_glyphs.clear();
}

void FontAtlas::createBitmapFont() {
    // Create a simple monospace bitmap font for printable ASCII characters (32-126)
    // Each character is 8x16 pixels
    
    const int charWidth = 8;
    const int charHeight = 16;
    const int charsPerRow = 16;
    const int numRows = 6; // 95 printable ASCII chars
    
    m_atlasWidth = charsPerRow * charWidth;
    m_atlasHeight = numRows * charHeight;
    m_lineHeight = (float)charHeight;
    
    // Create bitmap data (simple white on black for now)
    std::vector<unsigned char> bitmap(m_atlasWidth * m_atlasHeight * 4, 0);
    
    // Fill with a simple pattern for each character
    // This is a very basic placeholder - in a real implementation, you'd load actual font data
    for (int charIdx = 0; charIdx < 95; ++charIdx) {
        char c = (char)(32 + charIdx);
        
        int row = charIdx / charsPerRow;
        int col = charIdx % charsPerRow;
        
        int baseX = col * charWidth;
        int baseY = row * charHeight;
        
        // Draw a simple pattern for this character (vertical lines for visibility)
        for (int y = 0; y < charHeight; ++y) {
            for (int x = 0; x < charWidth; ++x) {
                int pixelX = baseX + x;
                int pixelY = baseY + y;
                int pixelIdx = (pixelY * m_atlasWidth + pixelX) * 4;
                
                // Create a simple pattern based on character
                bool pixel = false;
                if (c >= '0' && c <= '9') {
                    // Numbers: draw a box
                    pixel = (x == 0 || x == charWidth-1 || y == 0 || y == charHeight-1);
                } else if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
                    // Letters: draw vertical lines
                    pixel = (x == 1 || x == charWidth-2);
                } else {
                    // Other chars: draw a dot
                    pixel = (x == charWidth/2 && y == charHeight/2);
                }
                
                if (pixel) {
                    bitmap[pixelIdx + 0] = 255; // R
                    bitmap[pixelIdx + 1] = 255; // G
                    bitmap[pixelIdx + 2] = 255; // B
                    bitmap[pixelIdx + 3] = 255; // A
                }
            }
        }
        
        // Store glyph info
        Glyph glyph;
        glyph.uvMin = glm::vec2((float)baseX / m_atlasWidth, (float)baseY / m_atlasHeight);
        glyph.uvMax = glm::vec2((float)(baseX + charWidth) / m_atlasWidth, (float)(baseY + charHeight) / m_atlasHeight);
        glyph.size = glm::vec2((float)charWidth, (float)charHeight);
        glyph.bearing = glm::vec2(0, 0);
        glyph.advance = (float)charWidth;
        
        m_glyphs[c] = glyph;
    }
    
    // Create OpenGL texture
    GLCall(glGenTextures(1, &m_textureID));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_textureID));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_atlasWidth, m_atlasHeight, 0, 
                        GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data()));
    
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

const Glyph* FontAtlas::getGlyph(char c) const {
    auto it = m_glyphs.find(c);
    if (it != m_glyphs.end()) {
        return &it->second;
    }
    
    // Return space character as fallback
    auto spaceIt = m_glyphs.find(' ');
    if (spaceIt != m_glyphs.end()) {
        return &spaceIt->second;
    }
    
    return nullptr;
}

glm::vec2 FontAtlas::measureText(const char* text) const {
    if (!text || !text[0]) {
        return glm::vec2(0, 0);
    }
    
    float width = 0.0f;
    float height = m_lineHeight;
    
    while (*text) {
        const Glyph* glyph = getGlyph(*text);
        if (glyph) {
            width += glyph->advance;
        }
        ++text;
    }
    
    return glm::vec2(width, height);
}

} // namespace EngineUI
