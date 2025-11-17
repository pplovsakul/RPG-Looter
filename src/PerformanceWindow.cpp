#include "PerformanceWindow.h"
#include <algorithm>
#include <numeric>

void PerformanceWindow::update(EntityManager& em, float deltaTime) {
    auto& settings = GlobalSettings::getInstance();
    
    // Only show if the window is visible
    if (!settings.windowVisibility.showPerformanceWindow) {
        return;
    }
    
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("Performance Monitor", &settings.windowVisibility.showPerformanceWindow)) {
        ImGui::End();
        return;
    }
    
    // Quick stats at the top
    float fps = ImGui::GetIO().Framerate;
    float frameTime = 1000.0f / fps;
    
    ImGui::Text("FPS: %.1f (%.2f ms)", fps, frameTime);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "| Delta: %.4f s", deltaTime);
    
    // Entity count
    entityCount = em.getAllEntities().size();
    ImGui::Text("Entities: %zu", entityCount);
    
    ImGui::Separator();
    
    // Collapsible sections
    if (ImGui::CollapsingHeader("Performance Graph", ImGuiTreeNodeFlags_DefaultOpen)) {
        drawPerformanceGraph();
    }
    
    if (ImGui::CollapsingHeader("Detailed Statistics")) {
        drawDetailedStats(em, deltaTime);
    }
    
    if (ImGui::CollapsingHeader("System Controls")) {
        drawSystemControls();
    }
    
    ImGui::End();
}

void PerformanceWindow::recordFrameTime(float frameTime) {
    frameTimeHistory.push_back(frameTime);
    if (frameTimeHistory.size() > maxHistorySize) {
        frameTimeHistory.pop_front();
    }
    
    float fps = 1000.0f / frameTime;
    fpsHistory.push_back(fps);
    if (fpsHistory.size() > maxHistorySize) {
        fpsHistory.pop_front();
    }
    
    // Update stats
    if (!frameTimeHistory.empty()) {
        avgFrameTime = std::accumulate(frameTimeHistory.begin(), frameTimeHistory.end(), 0.0f) / frameTimeHistory.size();
        minFrameTime = *std::min_element(frameTimeHistory.begin(), frameTimeHistory.end());
        maxFrameTime = *std::max_element(frameTimeHistory.begin(), frameTimeHistory.end());
    }
}

void PerformanceWindow::drawPerformanceGraph() {
    if (!frameTimeHistory.empty()) {
        // Convert deque to vector for ImGui
        std::vector<float> frameTimesVec(frameTimeHistory.begin(), frameTimeHistory.end());
        
        ImGui::PlotLines("Frame Time (ms)", 
            frameTimesVec.data(), 
            frameTimesVec.size(), 
            0, 
            nullptr, 
            0.0f, 
            maxFrameTime * 1.2f, 
            ImVec2(0, 80));
        
        std::vector<float> fpsVec(fpsHistory.begin(), fpsHistory.end());
        ImGui::PlotHistogram("FPS", 
            fpsVec.data(), 
            fpsVec.size(), 
            0, 
            nullptr, 
            0.0f, 
            200.0f, 
            ImVec2(0, 80));
    }
}

void PerformanceWindow::drawDetailedStats(EntityManager& em, float deltaTime) {
    ImGui::Text("Frame Time Statistics:");
    ImGui::Indent();
    ImGui::Text("Average: %.2f ms", avgFrameTime);
    ImGui::Text("Min: %.2f ms", minFrameTime);
    ImGui::Text("Max: %.2f ms", maxFrameTime);
    ImGui::Unindent();
    
    ImGui::Spacing();
    ImGui::Text("Entity Information:");
    ImGui::Indent();
    
    // Count entities by components
    int withTransform = 0;
    int withRender = 0;
    int withAudio = 0;
    int withModel = 0;
    
    for (auto* entity : em.getAllEntities()) {
        if (entity->hasComponent<TransformComponent>()) withTransform++;
        if (entity->hasComponent<RenderComponent>()) withRender++;
        if (entity->hasComponent<AudioComponent>()) withAudio++;
        if (entity->hasComponent<ModelComponent>()) withModel++;
    }
    
    ImGui::Text("Total Entities: %zu", entityCount);
    ImGui::Text("With Transform: %d", withTransform);
    ImGui::Text("With Render: %d", withRender);
    ImGui::Text("With Audio: %d", withAudio);
    ImGui::Text("With Model: %d", withModel);
    ImGui::Unindent();
}

void PerformanceWindow::drawSystemControls() {
    auto& settings = GlobalSettings::getInstance();
    
    ImGui::Text("System Toggles:");
    ImGui::Spacing();
    
    if (ImGui::Checkbox("Rendering", &settings.systemControls.renderingEnabled)) {
        if (settings.systemControls.renderingEnabled) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Rendering enabled");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Rendering disabled");
        }
    }
    
    if (ImGui::Checkbox("Audio", &settings.systemControls.audioEnabled)) {
        if (settings.systemControls.audioEnabled) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Audio enabled");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Audio disabled");
        }
    }
    
    if (ImGui::Checkbox("Physics", &settings.systemControls.physicsEnabled)) {
        if (settings.systemControls.physicsEnabled) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Physics enabled");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Physics disabled");
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("VSync:");
    if (ImGui::Checkbox("Enable VSync", &settings.renderingSettings.vsyncEnabled)) {
        // Note: Will be applied in main loop
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
        "System controls are now functional!\n"
        "These flags control system behavior.");
}
