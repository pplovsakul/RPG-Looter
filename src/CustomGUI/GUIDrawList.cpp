#include "GUIDrawList.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace CustomGUI {

static const float GIM_PI = 3.14159265358979323846f;

GUIDrawList::GUIDrawList() 
    : clipRectMin(0, 0), clipRectMax(10000, 10000), currentTextureId(0) {
}

void GUIDrawList::clear() {
    vertices.clear();
    indices.clear();
    commands.clear();
    path.clear();
    clipRectMin = Vec2(0, 0);
    clipRectMax = Vec2(10000, 10000);
    currentTextureId = 0;
}

void GUIDrawList::pushClipRect(const Vec2& min, const Vec2& max) {
    clipRectMin = min;
    clipRectMax = max;
    onChangedClipRect();
}

void GUIDrawList::popClipRect() {
    // Reset to full screen - in a real implementation, this would use a stack
    clipRectMin = Vec2(0, 0);
    clipRectMax = Vec2(10000, 10000);
    onChangedClipRect();
}

void GUIDrawList::pushTextureID(unsigned int textureId) {
    currentTextureId = textureId;
    onChangedTextureID();
}

void GUIDrawList::popTextureID() {
    currentTextureId = 0;
    onChangedTextureID();
}

void GUIDrawList::addLine(const Vec2& p1, const Vec2& p2, const Color& col, float thickness) {
    if ((col.a == 0.0f) || (thickness < 0.01f))
        return;
    
    pathLineTo(p1);
    pathLineTo(p2);
    pathStroke(col, false, thickness);
}

void GUIDrawList::addRect(const Vec2& min, const Vec2& max, const Color& col, float rounding, float thickness) {
    if ((col.a == 0.0f) || (thickness < 0.01f))
        return;
    
    pathRect(min, max, rounding);
    pathStroke(col, true, thickness);
}

void GUIDrawList::addRectFilled(const Vec2& min, const Vec2& max, const Color& col, float rounding) {
    if (col.a == 0.0f)
        return;
    
    if (rounding > 0.0f) {
        pathRect(min, max, rounding);
        pathFillConvex(col);
    } else {
        primReserve(6, 4);
        primRect(min, Vec2(max.x, min.y), max, Vec2(min.x, max.y), col);
    }
}

void GUIDrawList::addRectFilledMultiColor(const Vec2& min, const Vec2& max, 
                                          const Color& colTopLeft, const Color& colTopRight,
                                          const Color& colBottomRight, const Color& colBottomLeft) {
    primReserve(6, 4);
    
    unsigned int idx = static_cast<unsigned int>(vertices.size());
    indices.push_back(idx); indices.push_back(idx + 1); indices.push_back(idx + 2);
    indices.push_back(idx); indices.push_back(idx + 2); indices.push_back(idx + 3);
    
    vertices.push_back({min, Vec2(0, 0), colTopLeft});
    vertices.push_back({Vec2(max.x, min.y), Vec2(1, 0), colTopRight});
    vertices.push_back({max, Vec2(1, 1), colBottomRight});
    vertices.push_back({Vec2(min.x, max.y), Vec2(0, 1), colBottomLeft});
}

void GUIDrawList::addTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Color& col, float thickness) {
    if ((col.a == 0.0f) || (thickness < 0.01f))
        return;
    
    pathLineTo(p1);
    pathLineTo(p2);
    pathLineTo(p3);
    pathStroke(col, true, thickness);
}

void GUIDrawList::addTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Color& col) {
    if (col.a == 0.0f)
        return;
    
    pathLineTo(p1);
    pathLineTo(p2);
    pathLineTo(p3);
    pathFillConvex(col);
}

void GUIDrawList::addCircle(const Vec2& center, float radius, const Color& col, int segments, float thickness) {
    if ((col.a == 0.0f) || (thickness < 0.01f) || (radius < 0.5f))
        return;
    
    if (segments == 0)
        segments = calcCircleAutoSegmentCount(radius);
    
    const float aMax = GIM_PI * 2.0f * ((float)segments - 1.0f) / (float)segments;
    pathArcTo(center, radius - 0.5f, 0.0f, aMax, segments);
    pathStroke(col, true, thickness);
}

void GUIDrawList::addCircleFilled(const Vec2& center, float radius, const Color& col, int segments) {
    if ((col.a == 0.0f) || (radius < 0.5f))
        return;
    
    if (segments == 0)
        segments = calcCircleAutoSegmentCount(radius);
    
    const float aMax = GIM_PI * 2.0f * ((float)segments - 1.0f) / (float)segments;
    pathArcTo(center, radius, 0.0f, aMax, segments);
    pathFillConvex(col);
}

void GUIDrawList::addText(const Vec2& pos, const Color& col, const char* text) {
    if (!text || !*text || col.a == 0.0f)
        return;
    
    // Simple text rendering - would integrate with font system in full implementation
    // For now, just draw a placeholder rectangle
    float textWidth = std::strlen(text) * 8.0f;  // Approximate
    float textHeight = 16.0f;
    addRectFilled(pos, Vec2(pos.x + textWidth, pos.y + textHeight), col);
}

void GUIDrawList::addImage(unsigned int textureId, const Vec2& min, const Vec2& max, 
                           const Vec2& uvMin, const Vec2& uvMax, const Color& col) {
    if (col.a == 0.0f)
        return;
    
    pushTextureID(textureId);
    primReserve(6, 4);
    primRectUV(min, Vec2(max.x, min.y), max, Vec2(min.x, max.y),
               uvMin, Vec2(uvMax.x, uvMin.y), uvMax, Vec2(uvMin.x, uvMax.y), col);
    popTextureID();
}

void GUIDrawList::pathClear() {
    path.clear();
}

void GUIDrawList::pathLineTo(const Vec2& pos) {
    path.push_back(pos);
}

void GUIDrawList::pathLineToMergeDuplicate(const Vec2& pos) {
    if (path.empty() || path.back().x != pos.x || path.back().y != pos.y)
        path.push_back(pos);
}

void GUIDrawList::pathFillConvex(const Color& col) {
    if (path.size() < 3)
        return;
    
    const int pointCount = static_cast<int>(path.size());
    primReserve((pointCount - 2) * 3, pointCount);
    
    unsigned int vtxIdx = static_cast<unsigned int>(vertices.size());
    for (int i = 0; i < pointCount; i++) {
        vertices.push_back({path[i], Vec2(0, 0), col});
    }
    
    for (int i = 2; i < pointCount; i++) {
        indices.push_back(vtxIdx);
        indices.push_back(vtxIdx + i - 1);
        indices.push_back(vtxIdx + i);
    }
    
    path.clear();
}

void GUIDrawList::pathStroke(const Color& col, bool closed, float thickness) {
    if (path.size() < 2)
        return;
    
    // Simplified stroke - just draw lines between points
    int pointCount = static_cast<int>(path.size());
    
    for (int i = 0; i < pointCount - 1; i++) {
        Vec2 p1 = path[i];
        Vec2 p2 = path[i + 1];
        Vec2 diff = p2 - p1;
        float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (len > 0.0f) {
            diff = diff / len;
            Vec2 perp(-diff.y, diff.x);
            perp = perp * (thickness * 0.5f);
            
            primReserve(6, 4);
            primRect(p1 + perp, p2 + perp, p2 - perp, p1 - perp, col);
        }
    }
    
    if (closed && pointCount > 2) {
        Vec2 p1 = path[pointCount - 1];
        Vec2 p2 = path[0];
        Vec2 diff = p2 - p1;
        float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (len > 0.0f) {
            diff = diff / len;
            Vec2 perp(-diff.y, diff.x);
            perp = perp * (thickness * 0.5f);
            
            primReserve(6, 4);
            primRect(p1 + perp, p2 + perp, p2 - perp, p1 - perp, col);
        }
    }
    
    path.clear();
}

void GUIDrawList::pathArcTo(const Vec2& center, float radius, float aMin, float aMax, int numSegments) {
    if (radius == 0.0f) {
        path.push_back(center);
        return;
    }
    
    if (numSegments == 0)
        numSegments = calcCircleAutoSegmentCount(radius);
    
    for (int i = 0; i <= numSegments; i++) {
        const float a = aMin + ((float)i / (float)numSegments) * (aMax - aMin);
        path.push_back(Vec2(center.x + std::cos(a) * radius, center.y + std::sin(a) * radius));
    }
}

void GUIDrawList::pathRect(const Vec2& min, const Vec2& max, float rounding) {
    if (rounding <= 0.0f) {
        pathLineTo(min);
        pathLineTo(Vec2(max.x, min.y));
        pathLineTo(max);
        pathLineTo(Vec2(min.x, max.y));
    } else {
        rounding = std::min(rounding, std::abs(max.x - min.x) * 0.5f);
        rounding = std::min(rounding, std::abs(max.y - min.y) * 0.5f);
        
        pathArcToFast(Vec2(min.x + rounding, min.y + rounding), rounding, 6, 9);
        pathArcToFast(Vec2(max.x - rounding, min.y + rounding), rounding, 9, 12);
        pathArcToFast(Vec2(max.x - rounding, max.y - rounding), rounding, 0, 3);
        pathArcToFast(Vec2(min.x + rounding, max.y - rounding), rounding, 3, 6);
    }
}

void GUIDrawList::primReserve(int idxCount, int vtxCount) {
    if (idxCount == 0 && vtxCount == 0)
        return;
    
    if (commands.empty() || commands.back().elemCount > 0) {
        commands.push_back(GUIDrawCmd());
        updateClipRect();
        updateTextureID();
    }
    
    vertices.reserve(vertices.size() + vtxCount);
    indices.reserve(indices.size() + idxCount);
}

void GUIDrawList::primRect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d, const Color& col) {
    unsigned int idx = static_cast<unsigned int>(vertices.size());
    indices.push_back(idx); indices.push_back(idx + 1); indices.push_back(idx + 2);
    indices.push_back(idx); indices.push_back(idx + 2); indices.push_back(idx + 3);
    vertices.push_back({a, Vec2(0, 0), col});
    vertices.push_back({b, Vec2(1, 0), col});
    vertices.push_back({c, Vec2(1, 1), col});
    vertices.push_back({d, Vec2(0, 1), col});
    commands.back().elemCount += 6;
}

void GUIDrawList::primRectUV(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d,
                             const Vec2& uvA, const Vec2& uvB, const Vec2& uvC, const Vec2& uvD, const Color& col) {
    unsigned int idx = static_cast<unsigned int>(vertices.size());
    indices.push_back(idx); indices.push_back(idx + 1); indices.push_back(idx + 2);
    indices.push_back(idx); indices.push_back(idx + 2); indices.push_back(idx + 3);
    vertices.push_back({a, uvA, col});
    vertices.push_back({b, uvB, col});
    vertices.push_back({c, uvC, col});
    vertices.push_back({d, uvD, col});
    commands.back().elemCount += 6;
}

void GUIDrawList::primQuadUV(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d,
                             const Vec2& uvA, const Vec2& uvB, const Vec2& uvC, const Vec2& uvD, const Color& col) {
    primRectUV(a, b, c, d, uvA, uvB, uvC, uvD, col);
}

void GUIDrawList::updateClipRect() {
    if (!commands.empty()) {
        commands.back().clipRect = Rect(clipRectMin, clipRectMax);
    }
}

void GUIDrawList::updateTextureID() {
    if (!commands.empty()) {
        commands.back().textureId = currentTextureId;
    }
}

void GUIDrawList::onChangedClipRect() {
    if (!commands.empty() && commands.back().elemCount == 0) {
        commands.back().clipRect = Rect(clipRectMin, clipRectMax);
    } else {
        GUIDrawCmd cmd;
        cmd.clipRect = Rect(clipRectMin, clipRectMax);
        cmd.textureId = currentTextureId;
        commands.push_back(cmd);
    }
}

void GUIDrawList::onChangedTextureID() {
    if (!commands.empty() && commands.back().elemCount == 0) {
        commands.back().textureId = currentTextureId;
    } else {
        GUIDrawCmd cmd;
        cmd.clipRect = Rect(clipRectMin, clipRectMax);
        cmd.textureId = currentTextureId;
        commands.push_back(cmd);
    }
}

int GUIDrawList::calcCircleAutoSegmentCount(float radius) const {
    const int segments = (int)(radius / 2.5f);
    return std::max(12, std::min(segments, 512));
}

void GUIDrawList::pathArcToFast(const Vec2& center, float radius, int aMinOf12, int aMaxOf12) {
    static const Vec2 circle12[12] = {
        Vec2(1.0f, 0.0f), Vec2(0.866f, 0.5f), Vec2(0.5f, 0.866f), Vec2(0.0f, 1.0f),
        Vec2(-0.5f, 0.866f), Vec2(-0.866f, 0.5f), Vec2(-1.0f, 0.0f), Vec2(-0.866f, -0.5f),
        Vec2(-0.5f, -0.866f), Vec2(0.0f, -1.0f), Vec2(0.5f, -0.866f), Vec2(0.866f, -0.5f)
    };
    
    for (int a = aMinOf12; a <= aMaxOf12; a++) {
        const Vec2& c = circle12[a % 12];
        path.push_back(Vec2(center.x + c.x * radius, center.y + c.y * radius));
    }
}

} // namespace CustomGUI
