#pragma once

#include <cstdint>
#include <vector>
#include "../vendor/glm/glm.hpp"

namespace EngineUI {

// ============================================================================
// Core Data Structures
// ============================================================================

struct Color {
    float r, g, b, a;
    
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    Color(uint32_t rgba);
    
    uint32_t toRGBA() const;
    
    static Color White() { return Color(1, 1, 1, 1); }
    static Color Black() { return Color(0, 0, 0, 1); }
    static Color Red() { return Color(1, 0, 0, 1); }
    static Color Green() { return Color(0, 1, 0, 1); }
    static Color Blue() { return Color(0, 0, 1, 1); }
    static Color Yellow() { return Color(1, 1, 0, 1); }
    static Color Transparent() { return Color(0, 0, 0, 0); }
};

struct Rect {
    float x, y, w, h;
    
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
    
    float left() const { return x; }
    float right() const { return x + w; }
    float top() const { return y; }
    float bottom() const { return y + h; }
    
    bool contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
    
    bool contains(const glm::vec2& p) const {
        return contains(p.x, p.y);
    }
    
    Rect intersect(const Rect& other) const;
    bool overlaps(const Rect& other) const;
};

struct DrawVertex {
    glm::vec2 pos;
    glm::vec2 uv;
    Color color;
    
    DrawVertex() = default;
    DrawVertex(const glm::vec2& pos, const glm::vec2& uv, const Color& col)
        : pos(pos), uv(uv), color(col) {}
};

// ============================================================================
// Draw Commands
// ============================================================================

struct DrawCommand {
    unsigned int textureID;
    unsigned int idxOffset;
    unsigned int elemCount;
    Rect clipRect;
    
    DrawCommand()
        : textureID(0), idxOffset(0), elemCount(0), clipRect() {}
};

// ============================================================================
// Draw List - accumulates rendering commands
// ============================================================================

class DrawList {
public:
    DrawList();
    
    // Primitives
    void addRect(const Rect& rect, const Color& color, float rounding = 0.0f, float thickness = 1.0f);
    void addRectFilled(const Rect& rect, const Color& color, float rounding = 0.0f);
    void addLine(const glm::vec2& p1, const glm::vec2& p2, const Color& color, float thickness = 1.0f);
    void addText(const glm::vec2& pos, const Color& color, const char* text);
    void addTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const Color& color, float thickness = 1.0f);
    void addTriangleFilled(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const Color& color);
    void addCircle(const glm::vec2& center, float radius, const Color& color, int segments = 12, float thickness = 1.0f);
    void addCircleFilled(const glm::vec2& center, float radius, const Color& color, int segments = 12);
    
    // Advanced
    void addImage(unsigned int textureID, const Rect& rect, const Rect& uv = Rect(0, 0, 1, 1), const Color& tint = Color::White());
    void addPolyline(const glm::vec2* points, int count, const Color& color, bool closed, float thickness = 1.0f);
    
    // Clipping
    void pushClipRect(const Rect& rect);
    void popClipRect();
    
    // Access to raw data
    const std::vector<DrawVertex>& getVertices() const { return m_vertices; }
    const std::vector<unsigned int>& getIndices() const { return m_indices; }
    const std::vector<DrawCommand>& getCommands() const { return m_commands; }
    
    // Set font texture for text rendering
    void setFontTexture(unsigned int fontTexID) { m_fontTexture = fontTexID; }
    
    void clear();
    
private:
    std::vector<DrawVertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<DrawCommand> m_commands;
    std::vector<Rect> m_clipRectStack;
    unsigned int m_fontTexture = 0;
    
    void primReserve(int idxCount, int vtxCount);
    void primRect(const Rect& rect, const Color& color, const Rect& uv = Rect(0, 0, 1, 1));
    void updateCurrentCommand(unsigned int textureID);
    
    unsigned int m_currentIdx = 0;
};

} // namespace EngineUI
