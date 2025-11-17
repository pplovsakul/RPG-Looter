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
    
    // Define 5x7 bitmap patterns for all printable ASCII characters
    // Each byte represents one row of 5 pixels (bits 4-0)
    unsigned char patterns[128][7];
    memset(patterns, 0, sizeof(patterns));
    
    // Uppercase letters A-Z
    unsigned char patternA[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    unsigned char patternB[7] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    unsigned char patternC[7] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
    unsigned char patternD[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
    unsigned char patternE[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    unsigned char patternF[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    unsigned char patternG[7] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F};
    unsigned char patternH[7] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    unsigned char patternI[7] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
    unsigned char patternJ[7] = {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C};
    unsigned char patternK[7] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    unsigned char patternL[7] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    unsigned char patternM[7] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
    unsigned char patternN[7] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    unsigned char patternO[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    unsigned char patternP[7] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    unsigned char patternQ[7] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
    unsigned char patternR[7] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    unsigned char patternS[7] = {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E};
    unsigned char patternT[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    unsigned char patternU[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    unsigned char patternV[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
    unsigned char patternW[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
    unsigned char patternX[7] = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
    unsigned char patternY[7] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    unsigned char patternZ[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
    
    // Numbers 0-9
    unsigned char pattern0[7] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    unsigned char pattern1[7] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    unsigned char pattern2[7] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    unsigned char pattern3[7] = {0x0E, 0x11, 0x01, 0x0E, 0x01, 0x11, 0x0E};
    unsigned char pattern4[7] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    unsigned char pattern5[7] = {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E};
    unsigned char pattern6[7] = {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    unsigned char pattern7[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    unsigned char pattern8[7] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    unsigned char pattern9[7] = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};
    
    // Common punctuation and symbols
    unsigned char patternSpace[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char patternExclaim[7] = {0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x04};
    unsigned char patternQuote[7] = {0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00};
    unsigned char patternHash[7] = {0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A};
    unsigned char patternPercent[7] = {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03};
    unsigned char patternAnd[7] = {0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D};
    unsigned char patternParenL[7] = {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02};
    unsigned char patternParenR[7] = {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08};
    unsigned char patternAsterisk[7] = {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00};
    unsigned char patternPlus[7] = {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00};
    unsigned char patternComma[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08};
    unsigned char patternMinus[7] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
    unsigned char patternDot[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
    unsigned char patternSlash[7] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00};
    unsigned char patternColon[7] = {0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00};
    unsigned char patternSemicolon[7] = {0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x08};
    unsigned char patternLess[7] = {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02};
    unsigned char patternEqual[7] = {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00};
    unsigned char patternGreater[7] = {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08};
    unsigned char patternQuestion[7] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04};
    unsigned char patternAt[7] = {0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E};
    unsigned char patternBracketL[7] = {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E};
    unsigned char patternBackslash[7] = {0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00};
    unsigned char patternBracketR[7] = {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E};
    unsigned char patternCaret[7] = {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00};
    unsigned char patternUnderscore[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
    unsigned char patternBacktick[7] = {0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00};
    unsigned char patternBraceL[7] = {0x06, 0x04, 0x04, 0x08, 0x04, 0x04, 0x06};
    unsigned char patternPipe[7] = {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    unsigned char patternBraceR[7] = {0x0C, 0x04, 0x04, 0x02, 0x04, 0x04, 0x0C};
    unsigned char patternTilde[7] = {0x00, 0x00, 0x08, 0x15, 0x02, 0x00, 0x00};
    
    // Copy patterns into array
    memcpy(patterns['A'], patternA, 7); memcpy(patterns['B'], patternB, 7);
    memcpy(patterns['C'], patternC, 7); memcpy(patterns['D'], patternD, 7);
    memcpy(patterns['E'], patternE, 7); memcpy(patterns['F'], patternF, 7);
    memcpy(patterns['G'], patternG, 7); memcpy(patterns['H'], patternH, 7);
    memcpy(patterns['I'], patternI, 7); memcpy(patterns['J'], patternJ, 7);
    memcpy(patterns['K'], patternK, 7); memcpy(patterns['L'], patternL, 7);
    memcpy(patterns['M'], patternM, 7); memcpy(patterns['N'], patternN, 7);
    memcpy(patterns['O'], patternO, 7); memcpy(patterns['P'], patternP, 7);
    memcpy(patterns['Q'], patternQ, 7); memcpy(patterns['R'], patternR, 7);
    memcpy(patterns['S'], patternS, 7); memcpy(patterns['T'], patternT, 7);
    memcpy(patterns['U'], patternU, 7); memcpy(patterns['V'], patternV, 7);
    memcpy(patterns['W'], patternW, 7); memcpy(patterns['X'], patternX, 7);
    memcpy(patterns['Y'], patternY, 7); memcpy(patterns['Z'], patternZ, 7);
    
    // Lowercase = same as uppercase (simplified)
    for (char c = 'a'; c <= 'z'; ++c) {
        memcpy(patterns[c], patterns[c - 32], 7);
    }
    
    // Numbers
    memcpy(patterns['0'], pattern0, 7); memcpy(patterns['1'], pattern1, 7);
    memcpy(patterns['2'], pattern2, 7); memcpy(patterns['3'], pattern3, 7);
    memcpy(patterns['4'], pattern4, 7); memcpy(patterns['5'], pattern5, 7);
    memcpy(patterns['6'], pattern6, 7); memcpy(patterns['7'], pattern7, 7);
    memcpy(patterns['8'], pattern8, 7); memcpy(patterns['9'], pattern9, 7);
    
    // Special characters
    memcpy(patterns[' '], patternSpace, 7);
    memcpy(patterns['!'], patternExclaim, 7);
    memcpy(patterns['"'], patternQuote, 7);
    memcpy(patterns['#'], patternHash, 7);
    memcpy(patterns['%'], patternPercent, 7);
    memcpy(patterns['&'], patternAnd, 7);
    memcpy(patterns['('], patternParenL, 7);
    memcpy(patterns[')'], patternParenR, 7);
    memcpy(patterns['*'], patternAsterisk, 7);
    memcpy(patterns['+'], patternPlus, 7);
    memcpy(patterns[','], patternComma, 7);
    memcpy(patterns['-'], patternMinus, 7);
    memcpy(patterns['.'], patternDot, 7);
    memcpy(patterns['/'], patternSlash, 7);
    memcpy(patterns[':'], patternColon, 7);
    memcpy(patterns[';'], patternSemicolon, 7);
    memcpy(patterns['<'], patternLess, 7);
    memcpy(patterns['='], patternEqual, 7);
    memcpy(patterns['>'], patternGreater, 7);
    memcpy(patterns['?'], patternQuestion, 7);
    memcpy(patterns['@'], patternAt, 7);
    memcpy(patterns['['], patternBracketL, 7);
    memcpy(patterns['\\'], patternBackslash, 7);
    memcpy(patterns[']'], patternBracketR, 7);
    memcpy(patterns['^'], patternCaret, 7);
    memcpy(patterns['_'], patternUnderscore, 7);
    memcpy(patterns['`'], patternBacktick, 7);
    memcpy(patterns['{'], patternBraceL, 7);
    memcpy(patterns['|'], patternPipe, 7);
    memcpy(patterns['}'], patternBraceR, 7);
    memcpy(patterns['~'], patternTilde, 7);
    
    // Default pattern for unsupported chars
    unsigned char patternDefault[7] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F};
    
    // Fill atlas with characters
    for (int charIdx = 0; charIdx < 95; ++charIdx) {
        char c = (char)(32 + charIdx);
        
        int row = charIdx / charsPerRow;
        int col = charIdx % charsPerRow;
        
        int baseX = col * charWidth;
        int baseY = row * charHeight;
        
        // Select pattern based on character
        const unsigned char* pattern = patterns[(unsigned char)c];
        
        // If no pattern defined, use default box
        bool hasPattern = false;
        for (int i = 0; i < 7; ++i) {
            if (pattern[i] != 0) {
                hasPattern = true;
                break;
            }
        }
        
        if (!hasPattern && c != ' ') {
            pattern = patternDefault;
        }
        
        // Draw the character pattern
        drawChar(baseX, baseY, pattern, 7);
        
        // Store glyph info
        Glyph glyph;
        // Flip Y coordinates for OpenGL (texture Y=0 is at bottom, but bitmap Y=0 is at top)
        float uvMinY = 1.0f - (float)(baseY + charHeight) / m_atlasHeight;
        float uvMaxY = 1.0f - (float)baseY / m_atlasHeight;
        glyph.uvMin = glm::vec2((float)baseX / m_atlasWidth, uvMinY);
        glyph.uvMax = glm::vec2((float)(baseX + charWidth) / m_atlasWidth, uvMaxY);
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
