#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <AL/al.h>
#include <AL/alc.h>

struct WAVData {
    int channels = 0;          // Anzahl Kanäle (1 = mono, 2 = stereo)
    int sampleRate = 0;        // Sample-Rate (z.B. 44100)
    int bitsPerSample = 0;     // Bits pro Sample (8 oder 16)
    std::vector<char> pcmData; // rohe PCM-Daten (Byte-Layout abhängig von bitsPerSample)
};

/// WavLoader utility.
/// LoadWav(path, out, optErr) liest ein WAV (PCM) und füllt out.
/// Rückgabe: true = erfolgreich, false = Fehler (optErr gefüllt falls != nullptr).
class WavLoader {
public:
    static bool LoadWav(const std::string& path, WAVData& out, std::string* outError = nullptr);
};

class Sound {
public:
    Sound() = default;
    ~Sound();

    // Lade WAV von Datei und erstelle OpenAL Buffer+Source
    bool LoadFromFile(const std::string& path, std::string* outError = nullptr);

    // Abspielen (mehrfaches Abspielen überschreibt den aktuellen Startpunkt)
    void Play();

    // Stoppen
    void Stop();

    // Lautstärke (0.0 = stumm, 1.0 = normal, >1.0 = lauter)
    void SetVolume(float volume);

    void SetPitch(float pitch);

    // Wiederholung (Loop an/aus)
    void SetLooping(bool loop);

    // Zugriff auf Source-ID (z.B. für advanced AL-Einstellungen)
    ALuint GetSource() const { return source; }

private:
    ALuint buffer = 0;
    ALuint source = 0;
};