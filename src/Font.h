#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class Texture;

struct GlyphMetrics {
    int unicode;
    char character;
    float advance;
    
    // Plane bounds (logical glyph bounds)
    float planeBounds[4]; // left, bottom, right, top
    
    // Atlas bounds (texture coordinates in pixels)
    float atlasBounds[4]; // left, bottom, right, top
};

struct KerningPair {
    int first;
    int second;
    float advance;
};

class Font {
private:
    std::string m_Name;
    std::unique_ptr<Texture> m_AtlasTexture;
    
    // Font metadata
    int m_AtlasWidth;
    int m_AtlasHeight;
    float m_LineHeight;
    float m_Base;
    int m_Size;
    int m_DistanceRange;
    
    // Glyph data
    std::vector<GlyphMetrics> m_Glyphs;
    std::unordered_map<int, size_t> m_UnicodeToGlyphIndex;
    
    // Kerning data
    std::vector<KerningPair> m_KerningPairs;
    std::unordered_map<uint64_t, float> m_KerningMap; // (first << 32 | second) -> advance

public:
    Font(const std::string& name, const std::string& jsonPath, const std::string& atlasPath);
    ~Font();

    // Get glyph metrics for a character
    const GlyphMetrics* getGlyph(int unicode) const;
    
    // Get kerning adjustment between two characters
    float getKerning(int first, int second) const;
    
    // Get font metrics
    float getLineHeight() const { return m_LineHeight; }
    float getBase() const { return m_Base; }
    int getSize() const { return m_Size; }
    int getDistanceRange() const { return m_DistanceRange; }
    
    // Get atlas dimensions
    int getAtlasWidth() const { return m_AtlasWidth; }
    int getAtlasHeight() const { return m_AtlasHeight; }
    
    // Bind atlas texture
    Texture* getAtlasTexture() const { return m_AtlasTexture.get(); }
    
    const std::string& getName() const { return m_Name; }

private:
    bool loadMetrics(const std::string& jsonPath);
    void buildKerningMap();
};
