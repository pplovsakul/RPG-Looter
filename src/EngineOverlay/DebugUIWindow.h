#pragma once

#include "UIContext.h"

namespace EngineUI {

/**
 * Debug UI window - shows render state diagnostics
 */
class DebugUIWindow {
public:
    DebugUIWindow();
    
    void render(bool* p_open = nullptr);
    
private:
    // Track some debug metrics
    int m_drawCalls = 0;
    int m_vertices = 0;
    int m_indices = 0;
};

} // namespace EngineUI
