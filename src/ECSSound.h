#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <vector>

// WAV-Daten Struktur (aus Audiomanager.h)
struct WAVData {
    int channels = 0;
    int sampleRate = 0;
    int bitsPerSample = 0;
    std::vector<char> pcmData;
};

// WAV Loader (aus Audiomanager.cpp)
class WavLoader {
public:
    static bool LoadWav(const std::string& path, WAVData& out, std::string* outError = nullptr);
};

class ECSSound {
public:
    ECSSound() = default;
    ~ECSSound();

    bool LoadFromFile(const std::string& path, std::string* outError = nullptr);
    void Play();
    void Stop();
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLooping(bool loop);
    bool IsPlaying() const;

    ALuint GetSource() const { return source; }

private:
    ALuint buffer = 0;
    ALuint source = 0;
};

