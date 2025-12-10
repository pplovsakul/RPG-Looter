#pragma once

#include "Entitymanager.h"
#include "CameraController.h"
#include "RenderSystem3D.h"

class DebugWindow {
private:
    EntityManager* entityManager;
    CameraController* cameraController;
    RenderSystem3D* renderSystem;
    
    // UI State
    bool showPerformance;
    bool showCameraInfo;
    bool showEntityList;
    bool showRenderStats;
    bool showLightingControls;
    
    // Performance tracking
    float fpsHistory[100];
    int fpsHistoryIndex;
    float avgFPS;
    float lastFrameTime;
    
    Entity* selectedEntity;

public:
    DebugWindow(EntityManager* em, CameraController* camera, RenderSystem3D* renderer);
    
    void render(float deltaTime);
    
    void setShowPerformance(bool show) { showPerformance = show; }
    void setShowCameraInfo(bool show) { showCameraInfo = show; }
    void setShowEntityList(bool show) { showEntityList = show; }
    void setShowRenderStats(bool show) { showRenderStats = show; }
    void setShowLightingControls(bool show) { showLightingControls = show; }

private:
    void renderPerformanceWindow();
    void renderCameraWindow();
    void renderEntityListWindow();
    void renderRenderStatsWindow();
    void renderLightingControlsWindow();
    void renderEntityInspector();
    
    void updateFPSHistory(float fps);
};
