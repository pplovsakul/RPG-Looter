#include "Font.h"
#include "Texture.h"
#include "JsonParser.h"
#include <iostream>
#include <fstream>

Font::Font(const std::string& name, const std::string& jsonPath, const std::string& atlasPath)
    : m_Name(name), m_AtlasWidth(0), m_AtlasHeight(0), m_LineHeight(0), 
      m_Base(0), m_Size(0), m_DistanceRange(0)
{
    // Load the atlas texture
    try {
        m_AtlasTexture = std::make_unique<Texture>(atlasPath);
        std::cout << "[Font] Loaded atlas texture: " << atlasPath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Font] Failed to load atlas texture: " << atlasPath << " - " << e.what() << std::endl;
        return;
    }
    
    // Load font metrics from JSON
    if (!loadMetrics(jsonPath)) {
        std::cerr << "[Font] Failed to load font metrics: " << jsonPath << std::endl;
        return;
    }
    
    // Build kerning lookup map
    buildKerningMap();
    
    std::cout << "[Font] Successfully loaded font '" << name << "' with " 
              << m_Glyphs.size() << " glyphs" << std::endl;
}

Font::~Font() {
    // Texture will be automatically cleaned up by unique_ptr
}

const GlyphMetrics* Font::getGlyph(int unicode) const {
    auto it = m_UnicodeToGlyphIndex.find(unicode);
    if (it != m_UnicodeToGlyphIndex.end()) {
        return &m_Glyphs[it->second];
    }
    return nullptr;
}

float Font::getKerning(int first, int second) const {
    uint64_t key = (static_cast<uint64_t>(first) << 32) | static_cast<uint64_t>(second);
    auto it = m_KerningMap.find(key);
    if (it != m_KerningMap.end()) {
        return it->second;
    }
    return 0.0f;
}

bool Font::loadMetrics(const std::string& jsonPath) {
    try {
        // Parse JSON file
        JsonParser parser;
        JsonValue root = parser.parseFile(jsonPath);
        
        if (!root.isObject()) {
            std::cerr << "[Font] Root is not a JSON object" << std::endl;
            return false;
        }
        
        const auto& rootObj = root.asObject();
        
        // Parse atlas metadata
        if (rootObj.find("atlas") != rootObj.end() && rootObj.at("atlas").isObject()) {
            const auto& atlas = rootObj.at("atlas").asObject();
            
            if (atlas.find("width") != atlas.end())
                m_AtlasWidth = static_cast<int>(atlas.at("width").asNumber());
            if (atlas.find("height") != atlas.end())
                m_AtlasHeight = static_cast<int>(atlas.at("height").asNumber());
            if (atlas.find("size") != atlas.end())
                m_Size = static_cast<int>(atlas.at("size").asNumber());
            if (atlas.find("distanceRange") != atlas.end())
                m_DistanceRange = static_cast<int>(atlas.at("distanceRange").asNumber());
        }
        
        // Parse font metrics
        if (rootObj.find("metrics") != rootObj.end() && rootObj.at("metrics").isObject()) {
            const auto& metrics = rootObj.at("metrics").asObject();
            
            if (metrics.find("lineHeight") != metrics.end())
                m_LineHeight = static_cast<float>(metrics.at("lineHeight").asNumber());
            if (metrics.find("base") != metrics.end())
                m_Base = static_cast<float>(metrics.at("base").asNumber());
        }
        
        // Parse glyphs
        if (rootObj.find("glyphs") != rootObj.end() && rootObj.at("glyphs").isArray()) {
            const auto& glyphsArray = rootObj.at("glyphs").asArray();
            
            for (const auto& glyphValue : glyphsArray) {
                if (!glyphValue.isObject()) continue;
                
                const auto& glyphObj = glyphValue.asObject();
                
                GlyphMetrics glyph = {};
                
                if (glyphObj.find("unicode") != glyphObj.end())
                    glyph.unicode = static_cast<int>(glyphObj.at("unicode").asNumber());
                else
                    continue; // Skip glyphs without unicode
                
                glyph.character = static_cast<char>(glyph.unicode);
                
                if (glyphObj.find("advance") != glyphObj.end())
                    glyph.advance = static_cast<float>(glyphObj.at("advance").asNumber());
                
                // Parse plane bounds
                if (glyphObj.find("planeBounds") != glyphObj.end() && glyphObj.at("planeBounds").isObject()) {
                    const auto& planeBounds = glyphObj.at("planeBounds").asObject();
                    
                    if (planeBounds.find("left") != planeBounds.end())
                        glyph.planeBounds[0] = static_cast<float>(planeBounds.at("left").asNumber());
                    if (planeBounds.find("bottom") != planeBounds.end())
                        glyph.planeBounds[1] = static_cast<float>(planeBounds.at("bottom").asNumber());
                    if (planeBounds.find("right") != planeBounds.end())
                        glyph.planeBounds[2] = static_cast<float>(planeBounds.at("right").asNumber());
                    if (planeBounds.find("top") != planeBounds.end())
                        glyph.planeBounds[3] = static_cast<float>(planeBounds.at("top").asNumber());
                }
                
                // Parse atlas bounds
                if (glyphObj.find("atlasBounds") != glyphObj.end() && glyphObj.at("atlasBounds").isObject()) {
                    const auto& atlasBounds = glyphObj.at("atlasBounds").asObject();
                    
                    if (atlasBounds.find("left") != atlasBounds.end())
                        glyph.atlasBounds[0] = static_cast<float>(atlasBounds.at("left").asNumber());
                    if (atlasBounds.find("bottom") != atlasBounds.end())
                        glyph.atlasBounds[1] = static_cast<float>(atlasBounds.at("bottom").asNumber());
                    if (atlasBounds.find("right") != atlasBounds.end())
                        glyph.atlasBounds[2] = static_cast<float>(atlasBounds.at("right").asNumber());
                    if (atlasBounds.find("top") != atlasBounds.end())
                        glyph.atlasBounds[3] = static_cast<float>(atlasBounds.at("top").asNumber());
                }
                
                // Add to glyph list and index
                m_UnicodeToGlyphIndex[glyph.unicode] = m_Glyphs.size();
                m_Glyphs.push_back(glyph);
            }
        }
        
        // Parse kerning pairs (if present)
        if (rootObj.find("kerning") != rootObj.end() && rootObj.at("kerning").isArray()) {
            const auto& kerningArray = rootObj.at("kerning").asArray();
            
            for (const auto& kerningValue : kerningArray) {
                if (!kerningValue.isObject()) continue;
                
                const auto& kerningObj = kerningValue.asObject();
                
                KerningPair pair = {};
                
                if (kerningObj.find("first") != kerningObj.end())
                    pair.first = static_cast<int>(kerningObj.at("first").asNumber());
                if (kerningObj.find("second") != kerningObj.end())
                    pair.second = static_cast<int>(kerningObj.at("second").asNumber());
                if (kerningObj.find("advance") != kerningObj.end())
                    pair.advance = static_cast<float>(kerningObj.at("advance").asNumber());
                
                m_KerningPairs.push_back(pair);
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[Font] JSON parsing error: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

void Font::buildKerningMap() {
    m_KerningMap.clear();
    for (const auto& pair : m_KerningPairs) {
        uint64_t key = (static_cast<uint64_t>(pair.first) << 32) | static_cast<uint64_t>(pair.second);
        m_KerningMap[key] = pair.advance;
    }
}
