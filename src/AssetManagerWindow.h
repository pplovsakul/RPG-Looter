#pragma once
#include "System.h"
#include "AssetManager.h"
#include "vendor/imgui/imgui.h"
#include <string>
#include <vector>

class AssetManagerWindow : public System {
public:
    AssetManagerWindow() = default;
    void update(EntityManager& em, float deltaTime) override;

private:
    char newTextureName[128] = "texture_name";
    char newTexturePath[512] = "res/textures/";
    char newSoundName[128] = "sound_name";
    char newSoundPath[512] = "res/sounds/";
    
    // Search/filter
    char textureSearchBuffer[128] = "";
    char soundSearchBuffer[128] = "";
    char modelSearchBuffer[128] = "";

    // Stress test controls
    int cpuSpawnCount = 1000;
    int gpuSpawnCount = 1000;

    // Track spawned entities so we can clear them
    std::vector<unsigned int> spawnedCPUEntities;
    std::vector<unsigned int> spawnedGPUEntities;
    
    // View options
    int texturePreviewSize = 48;
    bool showAssetStats = true;
    
    void drawTextureSection(EntityManager& em);
    void drawSoundSection(EntityManager& em);
    void drawModelSection(EntityManager& em);
    void drawStressTestSection(EntityManager& em);
    void drawAssetStatistics();
};