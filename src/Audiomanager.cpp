#include "Audiomanager.h"
#include <fstream>
#include <stdexcept>
#include <cstring> // memcmp
#include <cstdint>

// Helfer zum Lesen kleiner Datentypen (little-endian)
static bool read_u32(std::ifstream& f, uint32_t& v) {
    return static_cast<bool>(f.read(reinterpret_cast<char*>(&v), sizeof(v)));
}
static bool read_u16(std::ifstream& f, uint16_t& v) {
    return static_cast<bool>(f.read(reinterpret_cast<char*>(&v), sizeof(v)));
}
static bool read_bytes(std::ifstream& f, char* buf, std::size_t n) {
    return static_cast<bool>(f.read(buf, n));
}

bool WavLoader::LoadWav(const std::string& path, WAVData& out, std::string* outError) {
    out = WAVData(); // reset

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        if (outError) *outError = "Could not open file: " + path;
        return false;
    }

    // --- RIFF Header (12 bytes) ---
    char riff[4];
    if (!read_bytes(file, riff, 4)) { if (outError) *outError = "Failed to read RIFF"; return false; }
    if (std::memcmp(riff, "RIFF", 4) != 0) { if (outError) *outError = "Not a RIFF file"; return false; }

    uint32_t riffChunkSize = 0;
    if (!read_u32(file, riffChunkSize)) { if (outError) *outError = "Failed to read RIFF size"; return false; }

    char wave[4];
    if (!read_bytes(file, wave, 4)) { if (outError) *outError = "Failed to read WAVE header"; return false; }
    if (std::memcmp(wave, "WAVE", 4) != 0) { if (outError) *outError = "Not a WAVE file"; return false; }

    // --- Chunks lesen ---
    bool gotFmt = false;
    bool gotData = false;

    // Temporäre fmt info
    uint16_t audioFormat = 0;
    uint16_t numChannels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;
    std::vector<char> dataBuffer;

    while (!file.eof()) {
        char chunkId[4];
        if (!read_bytes(file, chunkId, 4)) break; // Ende
        uint32_t chunkSize = 0;
        if (!read_u32(file, chunkSize)) { if (outError) *outError = "Failed to read chunk size"; return false; }

        // Normalize chunk id to string
        std::string id(chunkId, 4);

        if (id == "fmt ") {
            // fmt chunk
            gotFmt = true;
            // fmt chunk is usually 16 for PCM, but can be bigger if extra params present
            // read common fields first
            if (!read_u16(file, audioFormat)) { if (outError) *outError = "Failed to read audioFormat"; return false; }
            if (!read_u16(file, numChannels)) { if (outError) *outError = "Failed to read numChannels"; return false; }
            if (!read_u32(file, sampleRate)) { if (outError) *outError = "Failed to read sampleRate"; return false; }
            uint32_t byteRate = 0;
            if (!read_u32(file, byteRate)) { if (outError) *outError = "Failed to read byteRate"; return false; }
            uint16_t blockAlign = 0;
            if (!read_u16(file, blockAlign)) { if (outError) *outError = "Failed to read blockAlign"; return false; }
            if (!read_u16(file, bitsPerSample)) { if (outError) *outError = "Failed to read bitsPerSample"; return false; }

            // Wenn fmt chunk größer als 16, skip the remainder
            int bytesReadInFmt = 16; // we've read 16 bytes of fmt chunk fields after chunk header
            if (chunkSize > static_cast<uint32_t>(bytesReadInFmt)) {
                // skip extra fmt bytes
                file.ignore(static_cast<std::streamsize>(chunkSize - bytesReadInFmt));
            }
        }
        else if (id == "data") {
            // data chunk: PCM samples
            gotData = true;
            dataBuffer.resize(chunkSize);
            if (!read_bytes(file, dataBuffer.data(), chunkSize)) { if (outError) *outError = "Failed to read data chunk"; return false; }
        }
        else {
            // Unbekannter Chunk: skip
            file.ignore(static_cast<std::streamsize>(chunkSize));
        }

        // Align to word: if chunkSize is odd, a padding byte may be present
        if (chunkSize % 2 == 1) file.ignore(1);
    }

    if (!gotFmt) {
        if (outError) *outError = "Missing 'fmt ' chunk";
        return false;
    }
    if (!gotData) {
        if (outError) *outError = "Missing 'data' chunk";
        return false;
    }

    // Validierung
    if (audioFormat != 1) { // 1 = PCM
        if (outError) *outError = "Unsupported audio format (only PCM = 1 supported)";
        return false;
    }
    if (!(bitsPerSample == 8 || bitsPerSample == 16)) {
        if (outError) *outError = "Unsupported bits per sample (only 8 or 16 supported)";
        return false;
    }
    if (!(numChannels == 1 || numChannels == 2)) {
        if (outError) *outError = "Unsupported channel count (only mono or stereo supported)";
        return false;
    }

    // fill out structure
    out.channels = static_cast<int>(numChannels);
    out.sampleRate = static_cast<int>(sampleRate);
    out.bitsPerSample = static_cast<int>(bitsPerSample);
    out.pcmData = std::move(dataBuffer);

    return true;
}

Sound::~Sound() {
    if (source) alDeleteSources(1, &source);
    if (buffer) alDeleteBuffers(1, &buffer);
}

bool Sound::LoadFromFile(const std::string& path, std::string* outError) {
    WAVData wav;
    if (!WavLoader::LoadWav(path, wav, outError)) {
        return false;
    }

    // Bestimme das richtige OpenAL-Format
    ALenum format = 0;
    if (wav.channels == 1 && wav.bitsPerSample == 8) format = AL_FORMAT_MONO8;
    else if (wav.channels == 1 && wav.bitsPerSample == 16) format = AL_FORMAT_MONO16;
    else if (wav.channels == 2 && wav.bitsPerSample == 8) format = AL_FORMAT_STEREO8;
    else if (wav.channels == 2 && wav.bitsPerSample == 16) format = AL_FORMAT_STEREO16;
    else {
        if (outError) *outError = "Unsupported WAV format (channels/bits)";
        return false;
    }

    // Buffer und Source erzeugen
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, wav.pcmData.data(),
        static_cast<ALsizei>(wav.pcmData.size()), wav.sampleRate);

    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);

    return true;
}

void Sound::Play() {
    if (source) alSourcePlay(source);
}

void Sound::Stop() {
    if (source) alSourceStop(source);
}

void Sound::SetVolume(float volume) {
    if (source) alSourcef(source, AL_GAIN, volume);
}

void Sound::SetLooping(bool loop) {
    if (source) alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void Sound::SetPitch(float pitch) {
    if (source) alSourcef(source, AL_PITCH, pitch);
}