#pragma once
#include "System.h"
#include "AssetManager.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>

class AudioSystem : public System {
private:
    AssetManager* assetManager;

public:
    AudioSystem();
    ~AudioSystem();

    void init();
    void update(EntityManager& em, float deltaTime) override;
};