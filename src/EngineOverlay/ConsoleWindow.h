#pragma once

#include <string>
#include <vector>
#include <mutex>

namespace EngineUI {

// Forward declaration
struct Rect;

/**
 * Log entry for the console
 */
struct LogEntry {
    enum class Level {
        Info,
        Warning,
        Error
    };
    
    std::string message;
    Level level;
    float timestamp;
    
    LogEntry(const std::string& msg, Level lvl, float time)
        : message(msg), level(lvl), timestamp(time) {}
};

/**
 * Console window - displays rolling log with filtering
 */
class ConsoleWindow {
public:
    ConsoleWindow();
    
    void render(bool* p_open = nullptr, const Rect* customRect = nullptr);
    
    // Add log messages
    void addLog(const std::string& message, LogEntry::Level level = LogEntry::Level::Info);
    void clear();
    
    // Filter controls
    void setShowInfo(bool show) { m_showInfo = show; }
    void setShowWarning(bool show) { m_showWarning = show; }
    void setShowError(bool show) { m_showError = show; }
    
private:
    std::vector<LogEntry> m_logs;
    std::mutex m_logMutex;
    
    bool m_autoScroll = true;
    bool m_showInfo = true;
    bool m_showWarning = true;
    bool m_showError = true;
    
    float m_currentTime = 0.0f;
    
    static constexpr int MAX_LOGS = 1000;
};

} // namespace EngineUI
