#include "AudioSystem.h"
#include "Components.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include "ECSSound.h"

// ============================================================================
// AUDIO SYSTEM IMPLEMENTATION
// ============================================================================

AudioSystem::~AudioSystem() = default;

AudioSystem::AudioSystem() {
    assetManager = AssetManager::getInstance();
}


void AudioSystem::init() {
    std::cout << "[AudioSystem] Initializing...\n";
}

void AudioSystem::update(EntityManager& em, float dt) {
    auto entities = em.getEntitiesWith<AudioComponent>();

    for (Entity* e : entities) {
        auto audio = e->getComponent<AudioComponent>();
        if (!audio || !audio->enabled) continue;

        // Hole den Sound aus dem AssetManager
        ECSSound* sound = assetManager->getSound(audio->soundName);
        if (!sound) {
            std::cerr << "[AudioSystem] Warning: Sound not found in AssetManager: "
                << audio->soundName << "\n";
            continue;
        }

        // Setze Parameter
        sound->SetVolume(audio->volume);
        sound->SetPitch(audio->pitch);
        sound->SetLooping(audio->loop);

        // Falls play-Flag gesetzt ist
        if (audio->shouldPlay) {
            sound->Play();
            audio->shouldPlay = false;
            audio->isPlaying = true;
        }

        // Synchronisiere Play-Status zur�ck ins AudioComponent
        audio->isPlaying = sound->IsPlaying();
    }

}