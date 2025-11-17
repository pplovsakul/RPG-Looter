#pragma once
#include "System.h"
#include "Components.h"
#include "Entitymanager.h"
#include "vendor/imgui/imgui.h"
#include <vector>
#include <deque>
#include <string>

class PerformanceWindow : public System {
public:
    PerformanceWindow() = default;
    void update(EntityManager& em, float deltaTime) override;
    
    // Track frame times for performance graph
    void recordFrameTime(float frameTime);
    
private:
    // Performance tracking
    std::deque<float> frameTimeHistory;
    std::deque<float> fpsHistory;
    static constexpr int maxHistorySize = 120; // 2 seconds at 60fps
    
    // Stats
    float avgFrameTime = 0.0f;
    float minFrameTime = 999.0f;
    float maxFrameTime = 0.0f;
    
    // Memory tracking (simplified)
    size_t entityCount = 0;
    
    // UI state
    bool showGraph = true;
    bool showDetailedStats = true;
    bool showSystemControls = false;
    
    void drawPerformanceGraph();
    void drawDetailedStats(EntityManager& em, float deltaTime);
    void drawSystemControls();
};
