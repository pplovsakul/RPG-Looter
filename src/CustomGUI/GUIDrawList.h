#pragma once

#include "GUITypes.h"
#include <vector>
#include <cstdint>

namespace CustomGUI {

// Vertex structure for rendering
struct GUIVertex {
    Vec2 pos;
    Vec2 uv;
    Color col;
};

// Draw command - one draw call
struct GUIDrawCmd {
    unsigned int elemCount;     // Number of indices to be drawn
    Rect clipRect;              // Clipping rectangle
    unsigned int textureId;     // OpenGL texture ID (0 for colored primitives)
    
    GUIDrawCmd() : elemCount(0), clipRect(), textureId(0) {}
};

// Draw list - accumulates vertices and indices for rendering
class GUIDrawList {
private:
    std::vector<GUIVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<GUIDrawCmd> commands;
    
    Vec2 clipRectMin;
    Vec2 clipRectMax;
    unsigned int currentTextureId;
    
    // Path building
    std::vector<Vec2> path;
    
public:
    GUIDrawList();
    
    // Command management
    void clear();
    void pushClipRect(const Vec2& min, const Vec2& max);
    void popClipRect();
    void pushTextureID(unsigned int textureId);
    void popTextureID();
    
    // Primitive drawing
    void addLine(const Vec2& p1, const Vec2& p2, const Color& col, float thickness = 1.0f);
    void addRect(const Vec2& min, const Vec2& max, const Color& col, float rounding = 0.0f, float thickness = 1.0f);
    void addRectFilled(const Vec2& min, const Vec2& max, const Color& col, float rounding = 0.0f);
    void addRectFilledMultiColor(const Vec2& min, const Vec2& max, 
                                  const Color& colTopLeft, const Color& colTopRight,
                                  const Color& colBottomRight, const Color& colBottomLeft);
    void addTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Color& col, float thickness = 1.0f);
    void addTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Color& col);
    void addCircle(const Vec2& center, float radius, const Color& col, int segments = 0, float thickness = 1.0f);
    void addCircleFilled(const Vec2& center, float radius, const Color& col, int segments = 0);
    void addText(const Vec2& pos, const Color& col, const char* text);
    void addImage(unsigned int textureId, const Vec2& min, const Vec2& max, const Vec2& uvMin = Vec2(0,0), const Vec2& uvMax = Vec2(1,1), const Color& col = Color(1,1,1,1));
    
    // Path API (for custom shapes)
    void pathClear();
    void pathLineTo(const Vec2& pos);
    void pathLineToMergeDuplicate(const Vec2& pos);
    void pathFillConvex(const Color& col);
    void pathStroke(const Color& col, bool closed, float thickness = 1.0f);
    void pathArcTo(const Vec2& center, float radius, float aMin, float aMax, int numSegments = 0);
    void pathRect(const Vec2& min, const Vec2& max, float rounding = 0.0f);
    
    // Accessors
    const std::vector<GUIVertex>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }
    const std::vector<GUIDrawCmd>& getCommands() const { return commands; }
    
    std::vector<GUIVertex>& getVertices() { return vertices; }
    std::vector<unsigned int>& getIndices() { return indices; }
    std::vector<GUIDrawCmd>& getCommands() { return commands; }
    
    int getVertexCount() const { return static_cast<int>(vertices.size()); }
    int getIndexCount() const { return static_cast<int>(indices.size()); }
    int getCommandCount() const { return static_cast<int>(commands.size()); }
    
private:
    void primReserve(int idxCount, int vtxCount);
    void primRect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d, const Color& col);
    void primRectUV(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d, 
                    const Vec2& uvA, const Vec2& uvB, const Vec2& uvC, const Vec2& uvD, const Color& col);
    void primQuadUV(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d,
                    const Vec2& uvA, const Vec2& uvB, const Vec2& uvC, const Vec2& uvD, const Color& col);
    
    void updateClipRect();
    void updateTextureID();
    void onChangedClipRect();
    void onChangedTextureID();
    
    int calcCircleAutoSegmentCount(float radius) const;
    void pathArcToFast(const Vec2& center, float radius, int aMinOf12, int aMaxOf12);
};

} // namespace CustomGUI
