#pragma once

#include <functional>
#include <vector>
#include "vendor/glm/glm.hpp"
#include <GLFW/glfw3.h>

struct Vertex_Menu {
    glm::vec3 pos;
    glm::vec2 uv;
};

class Button {
public:
    // ctor takes bottom-left corner (x,y) and width/height in pixels (world/screen coords you use)
    Button(float x, float y, float w, float h);

    // callbacks
    void setOnClick(std::function<void()> func) { onClick = std::move(func); }
    void setOnHover(std::function<void(bool)> func) { onHover = std::move(func); } // arg = hovered

    // check click (edge-triggered). Returns true if a click was detected this call.
    // Must be called every frame to track mouse state (it updates internal previous state).
    bool isClicked(GLFWwindow* window);

    // convenience: returns true if cursor currently over button (non state-changing)
    bool IsHovered(GLFWwindow* window) const;

    // getters/setters for transform
    glm::vec2 get_position() const; // center (useful for RenderObject positioning)
    glm::vec2 get_size() const;
    void SetPosition(const glm::vec2& pos); // sets bottom-left corner (x,y)
    void SetSize(const glm::vec2& size);

    // Render data helpers (vertices centered around origin; use get_position() for placement)
    // Vertex layout: position(vec3), uv(vec2) — matches your Renderer default (3 + 2 floats)
    std::vector<Vertex_Menu> GetVertices() const;
    std::vector<unsigned int> GetIndices() const;
    std::vector<glm::vec2> GetTexCoords() const;

private:
    // hit test in same coordinate space as GetCursor (after y-flip)
    bool isHoveredInternal(double mouseX, double mouseY) const;

private:
    float x, y, width, height; // bottom-left origin
    std::function<void()> onClick;
    std::function<void(bool)> onHover;

    std::vector<unsigned int> m_indices;

    // internal state for edge detection
    bool prevMouseDown = false;
    bool prevHovered = false;
};