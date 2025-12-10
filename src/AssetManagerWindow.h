#pragma once
#include "System.h"
#include "AssetManager.h"
#include "GlobalSettings.h"
#include "vendor/imgui/imgui.h"
#include <string>
#include <vector>

class AssetManagerWindow : public System {
public:
    AssetManagerWindow() = default;
    void update(EntityManager& em, float deltaTime) override;

private:
    char newModelName[128] = "model_name";
    char newModelPath[512] = "res/models/";
    char newSoundName[128] = "sound_name";
    char newSoundPath[512] = "res/sounds/";
    
    // Search/filter
    char soundSearchBuffer[128] = "";
    char modelSearchBuffer[128] = "";

    // Stress test controls
    int cpuSpawnCount = 1000;
    int gpuSpawnCount = 1000;

    // Track spawned entities so we can clear them
    std::vector<unsigned int> spawnedCPUEntities;
    std::vector<unsigned int> spawnedGPUEntities;
    
    // View options
    bool showAssetStats = true;
    
    void drawSoundSection(EntityManager& em);
    void drawModelSection(EntityManager& em);
    void drawStressTestSection(EntityManager& em);
    void drawAssetStatistics();
};