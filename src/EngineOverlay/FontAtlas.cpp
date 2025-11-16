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
    // Each character is 8x16 pixels with readable 5x7 patterns
    
    const int charWidth = 8;
    const int charHeight = 16;
    const int charsPerRow = 16;
    const int numRows = 6; // 95 printable ASCII chars
    
    m_atlasWidth = charsPerRow * charWidth;
    m_atlasHeight = numRows * charHeight;
    m_lineHeight = (float)charHeight;
    
    // Create bitmap data (white on transparent background)
    std::vector<unsigned char> bitmap(m_atlasWidth * m_atlasHeight * 4, 0);
    
    // Simple 5x7 font patterns for basic ASCII characters
    // Each pattern is a 5-byte array representing 5x7 pixels (bits)
    auto drawChar = [&](int baseX, int baseY, const unsigned char* pattern, int patternHeight) {
        for (int y = 0; y < patternHeight && y < charHeight - 2; ++y) {
            unsigned char row = pattern[y];
            for (int x = 0; x < 5; ++x) {
                if (row & (1 << (4 - x))) {
                    int pixelX = baseX + x + 1; // Center in 8-wide cell
                    int pixelY = baseY + y + 4;  // Offset from top
                    int pixelIdx = (pixelY * m_atlasWidth + pixelX) * 4;
                    
                    bitmap[pixelIdx + 0] = 255; // R
                    bitmap[pixelIdx + 1] = 255; // G
                    bitmap[pixelIdx + 2] = 255; // B
                    bitmap[pixelIdx + 3] = 255; // A
                }
            }
        }
    };
    
    // Define simple patterns for common characters (5x7 bitmap)
    unsigned char patternSpace[7] = {0};
    unsigned char patternA[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    unsigned char patternB[7] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    unsigned char patternC[7] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
    unsigned char patternD[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
    unsigned char patternE[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    unsigned char patternF[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    unsigned char pattern0[7] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    unsigned char pattern1[7] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    unsigned char patternDot[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
    unsigned char patternColon[7] = {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00};
    unsigned char patternDefault[7] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F}; // Box for unknown chars
    
    // Fill atlas with characters
    for (int charIdx = 0; charIdx < 95; ++charIdx) {
        char c = (char)(32 + charIdx);
        
        int row = charIdx / charsPerRow;
        int col = charIdx % charsPerRow;
        
        int baseX = col * charWidth;
        int baseY = row * charHeight;
        
        // Select pattern based on character
        const unsigned char* pattern = patternDefault;
        if (c == ' ') pattern = patternSpace;
        else if (c == '.') pattern = patternDot;
        else if (c == ':') pattern = patternColon;
        else if (c >= 'A' && c <= 'F') pattern = (&patternA)[c - 'A'];
        else if (c >= 'a' && c <= 'f') pattern = (&patternA)[c - 'a']; // Reuse uppercase
        else if (c >= '0' && c <= '1') pattern = (&pattern0)[c - '0'];
        else {
            // For other letters and symbols, draw a filled rect to ensure visibility
            for (int y = 2; y < charHeight - 2; ++y) {
                for (int x = 1; x < charWidth - 1; ++x) {
                    int pixelX = baseX + x;
                    int pixelY = baseY + y;
                    int pixelIdx = (pixelY * m_atlasWidth + pixelX) * 4;
                    
                    // Draw outline or fill based on position
                    bool isBorder = (x == 1 || x == charWidth - 2 || y == 2 || y == charHeight - 3);
                    if (isBorder) {
                        bitmap[pixelIdx + 0] = 255;
                        bitmap[pixelIdx + 1] = 255;
                        bitmap[pixelIdx + 2] = 255;
                        bitmap[pixelIdx + 3] = 255;
                    }
                }
            }
        }
        
        // Draw the character pattern if we have one
        if (pattern != patternDefault || c == ' ') {
            drawChar(baseX, baseY, pattern, 7);
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
    
    // Use GL_UNPACK_ALIGNMENT=1 for proper texture upload
    GLCall(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_atlasWidth, m_atlasHeight, 0, 
                        GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data()));
    GLCall(glPixelStorei(GL_UNPACK_ALIGNMENT, 4)); // Restore default
    
    // Use NEAREST filtering for crisp bitmap font
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
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
