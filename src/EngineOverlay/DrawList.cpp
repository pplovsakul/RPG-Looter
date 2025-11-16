#include "DrawList.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace EngineUI {

// ============================================================================
// Color Implementation
// ============================================================================

Color::Color(uint32_t rgba) {
    r = ((rgba >> 24) & 0xFF) / 255.0f;
    g = ((rgba >> 16) & 0xFF) / 255.0f;
    b = ((rgba >> 8) & 0xFF) / 255.0f;
    a = (rgba & 0xFF) / 255.0f;
}

uint32_t Color::toRGBA() const {
    uint32_t rr = (uint32_t)(r * 255.0f) & 0xFF;
    uint32_t gg = (uint32_t)(g * 255.0f) & 0xFF;
    uint32_t bb = (uint32_t)(b * 255.0f) & 0xFF;
    uint32_t aa = (uint32_t)(a * 255.0f) & 0xFF;
    return (rr << 24) | (gg << 16) | (bb << 8) | aa;
}

// ============================================================================
// Rect Implementation
// ============================================================================

Rect Rect::intersect(const Rect& other) const {
    float l = std::max(left(), other.left());
    float t = std::max(top(), other.top());
    float r = std::min(right(), other.right());
    float b = std::min(bottom(), other.bottom());
    
    if (l >= r || t >= b) {
        return Rect(0, 0, 0, 0);
    }
    return Rect(l, t, r - l, b - t);
}

bool Rect::overlaps(const Rect& other) const {
    return !(right() <= other.left() || left() >= other.right() ||
             bottom() <= other.top() || top() >= other.bottom());
}

// ============================================================================
// DrawList Implementation
// ============================================================================

DrawList::DrawList() {
    m_vertices.reserve(1024);
    m_indices.reserve(2048);
    m_commands.reserve(16);
}

void DrawList::clear() {
    m_vertices.clear();
    m_indices.clear();
    m_commands.clear();
    m_clipRectStack.clear();
    m_currentIdx = 0;
}

void DrawList::primReserve(int idxCount, int vtxCount) {
    m_vertices.reserve(m_vertices.size() + vtxCount);
    m_indices.reserve(m_indices.size() + idxCount);
}

void DrawList::updateCurrentCommand(unsigned int textureID) {
    Rect clipRect;
    if (!m_clipRectStack.empty()) {
        clipRect = m_clipRectStack.back();
    }
    
    // Try to merge with previous command if possible
    if (!m_commands.empty()) {
        DrawCommand& cmd = m_commands.back();
        if (cmd.textureID == textureID && 
            cmd.clipRect.x == clipRect.x && cmd.clipRect.y == clipRect.y &&
            cmd.clipRect.w == clipRect.w && cmd.clipRect.h == clipRect.h) {
            // Can merge - just increment element count
            return;
        }
    }
    
    // Need new command
    DrawCommand cmd;
    cmd.textureID = textureID;
    cmd.idxOffset = m_indices.size();
    cmd.elemCount = 0;
    cmd.clipRect = clipRect;
    m_commands.push_back(cmd);
}

void DrawList::primRect(const Rect& rect, const Color& color, const Rect& uv) {
    unsigned int idx = m_vertices.size();
    
    m_vertices.push_back(DrawVertex(glm::vec2(rect.x, rect.y), glm::vec2(uv.x, uv.y), color));
    m_vertices.push_back(DrawVertex(glm::vec2(rect.x + rect.w, rect.y), glm::vec2(uv.x + uv.w, uv.y), color));
    m_vertices.push_back(DrawVertex(glm::vec2(rect.x + rect.w, rect.y + rect.h), glm::vec2(uv.x + uv.w, uv.y + uv.h), color));
    m_vertices.push_back(DrawVertex(glm::vec2(rect.x, rect.y + rect.h), glm::vec2(uv.x, uv.y + uv.h), color));
    
    m_indices.push_back(idx + 0);
    m_indices.push_back(idx + 1);
    m_indices.push_back(idx + 2);
    m_indices.push_back(idx + 0);
    m_indices.push_back(idx + 2);
    m_indices.push_back(idx + 3);
    
    if (!m_commands.empty()) {
        m_commands.back().elemCount += 6;
    }
}

void DrawList::addRectFilled(const Rect& rect, const Color& color, float rounding) {
    updateCurrentCommand(0);
    
    if (rounding <= 0.0f) {
        primReserve(6, 4);
        primRect(rect, color);
    } else {
        // TODO: Implement rounded rectangles
        primReserve(6, 4);
        primRect(rect, color);
    }
}

void DrawList::addRect(const Rect& rect, const Color& color, float rounding, float thickness) {
    updateCurrentCommand(0);
    
    if (rounding <= 0.0f && thickness <= 1.0f) {
        // Simple rect outline
        primReserve(24, 8);
        
        float t = thickness;
        // Top
        primRect(Rect(rect.x, rect.y, rect.w, t), color);
        // Bottom
        primRect(Rect(rect.x, rect.y + rect.h - t, rect.w, t), color);
        // Left
        primRect(Rect(rect.x, rect.y + t, t, rect.h - 2*t), color);
        // Right
        primRect(Rect(rect.x + rect.w - t, rect.y + t, t, rect.h - 2*t), color);
    } else {
        // TODO: Implement thick/rounded rect outlines
        addRect(rect, color, 0.0f, 1.0f);
    }
}

void DrawList::addLine(const glm::vec2& p1, const glm::vec2& p2, const Color& color, float thickness) {
    updateCurrentCommand(0);
    
    glm::vec2 dir = p2 - p1;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f) return;
    
    dir = dir / len;
    glm::vec2 perp(-dir.y * thickness * 0.5f, dir.x * thickness * 0.5f);
    
    primReserve(6, 4);
    
    unsigned int idx = m_vertices.size();
    m_vertices.push_back(DrawVertex(p1 - perp, glm::vec2(0, 0), color));
    m_vertices.push_back(DrawVertex(p2 - perp, glm::vec2(1, 0), color));
    m_vertices.push_back(DrawVertex(p2 + perp, glm::vec2(1, 1), color));
    m_vertices.push_back(DrawVertex(p1 + perp, glm::vec2(0, 1), color));
    
    m_indices.push_back(idx + 0);
    m_indices.push_back(idx + 1);
    m_indices.push_back(idx + 2);
    m_indices.push_back(idx + 0);
    m_indices.push_back(idx + 2);
    m_indices.push_back(idx + 3);
    
    if (!m_commands.empty()) {
        m_commands.back().elemCount += 6;
    }
}

void DrawList::addTriangleFilled(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const Color& color) {
    updateCurrentCommand(0);
    primReserve(3, 3);
    
    unsigned int idx = m_vertices.size();
    m_vertices.push_back(DrawVertex(p1, glm::vec2(0, 0), color));
    m_vertices.push_back(DrawVertex(p2, glm::vec2(0, 0), color));
    m_vertices.push_back(DrawVertex(p3, glm::vec2(0, 0), color));
    
    m_indices.push_back(idx + 0);
    m_indices.push_back(idx + 1);
    m_indices.push_back(idx + 2);
    
    if (!m_commands.empty()) {
        m_commands.back().elemCount += 3;
    }
}

void DrawList::addTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const Color& color, float thickness) {
    // Draw three lines
    addLine(p1, p2, color, thickness);
    addLine(p2, p3, color, thickness);
    addLine(p3, p1, color, thickness);
}

void DrawList::addCircleFilled(const glm::vec2& center, float radius, const Color& color, int segments) {
    updateCurrentCommand(0);
    
    if (segments < 3) segments = 12;
    
    primReserve(segments * 3, segments + 1);
    
    unsigned int centerIdx = m_vertices.size();
    m_vertices.push_back(DrawVertex(center, glm::vec2(0.5f, 0.5f), color));
    
    for (int i = 0; i < segments; ++i) {
        float angle = (float)i / (float)segments * 2.0f * 3.14159265f;
        glm::vec2 p(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
        m_vertices.push_back(DrawVertex(p, glm::vec2(0, 0), color));
    }
    
    for (int i = 0; i < segments; ++i) {
        m_indices.push_back(centerIdx);
        m_indices.push_back(centerIdx + 1 + i);
        m_indices.push_back(centerIdx + 1 + ((i + 1) % segments));
    }
    
    if (!m_commands.empty()) {
        m_commands.back().elemCount += segments * 3;
    }
}

void DrawList::addCircle(const glm::vec2& center, float radius, const Color& color, int segments, float thickness) {
    if (segments < 3) segments = 12;
    
    for (int i = 0; i < segments; ++i) {
        float angle1 = (float)i / (float)segments * 2.0f * 3.14159265f;
        float angle2 = (float)(i + 1) / (float)segments * 2.0f * 3.14159265f;
        glm::vec2 p1(center.x + std::cos(angle1) * radius, center.y + std::sin(angle1) * radius);
        glm::vec2 p2(center.x + std::cos(angle2) * radius, center.y + std::sin(angle2) * radius);
        addLine(p1, p2, color, thickness);
    }
}

void DrawList::addText(const glm::vec2& pos, const Color& color, const char* text) {
    if (!text || !text[0] || m_fontTexture == 0) return;
    
    // Note: We need the FontAtlas to render text properly
    // For now, this is a simplified version that won't work without FontAtlas access
    // The actual implementation should be done through UIContext
    
    // This is a placeholder - real implementation needs glyph data
    updateCurrentCommand(m_fontTexture);
}

void DrawList::addImage(unsigned int textureID, const Rect& rect, const Rect& uv, const Color& tint) {
    updateCurrentCommand(textureID);
    primReserve(6, 4);
    primRect(rect, tint, uv);
}

void DrawList::addPolyline(const glm::vec2* points, int count, const Color& color, bool closed, float thickness) {
    if (count < 2) return;
    
    for (int i = 0; i < count - 1; ++i) {
        addLine(points[i], points[i + 1], color, thickness);
    }
    
    if (closed && count > 2) {
        addLine(points[count - 1], points[0], color, thickness);
    }
}

void DrawList::pushClipRect(const Rect& rect) {
    if (!m_clipRectStack.empty()) {
        Rect intersected = m_clipRectStack.back().intersect(rect);
        m_clipRectStack.push_back(intersected);
    } else {
        m_clipRectStack.push_back(rect);
    }
}

void DrawList::popClipRect() {
    if (!m_clipRectStack.empty()) {
        m_clipRectStack.pop_back();
    }
}

} // namespace EngineUI
