#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "vendor/imgui/imgui.h"
#include <vector>
#include <string>
#include <chrono>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

struct LogEntry {
    std::string message;
    LogLevel level;
    std::chrono::system_clock::time_point timestamp;
    
    LogEntry(const std::string& msg, LogLevel lvl) 
        : message(msg), level(lvl), timestamp(std::chrono::system_clock::now()) {}
};

class ConsoleWindow : public System {
public:
    ConsoleWindow() = default;
    void update(EntityManager& em, float deltaTime) override;
    
    // Logging functions
    static void Log(const std::string& message, LogLevel level = LogLevel::Info);
    static void Info(const std::string& message) { Log(message, LogLevel::Info); }
    static void Warning(const std::string& message) { Log(message, LogLevel::Warning); }
    static void Error(const std::string& message) { Log(message, LogLevel::Error); }
    static void Debug(const std::string& message) { Log(message, LogLevel::Debug); }
    
    static void Clear();
    
private:
    static std::vector<LogEntry> logs;
    static constexpr int maxLogs = 1000;
    
    // Filters
    bool showInfo = true;
    bool showWarning = true;
    bool showError = true;
    bool showDebug = true;
    bool autoScroll = true;
    
    char inputBuffer[256] = "";
    
    void drawLogEntry(const LogEntry& entry, int index);
    ImVec4 getLevelColor(LogLevel level);
    const char* getLevelText(LogLevel level);
};
