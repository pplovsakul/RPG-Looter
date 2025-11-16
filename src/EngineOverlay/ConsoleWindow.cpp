#include "ConsoleWindow.h"
#include "Widgets.h"
#include "UIContext.h"
#include <cstdio>

namespace EngineUI {

ConsoleWindow::ConsoleWindow() {
    // Add some initial messages
    addLog("Console initialized", LogEntry::Level::Info);
}

void ConsoleWindow::render(bool* p_open) {
    UIContext* ctx = GetContext();
    if (!ctx) return;
    
    Rect initialRect(50, 400, 800, 300);
    if (!ctx->beginWindow("Console", p_open, &initialRect)) {
        return;
    }
    
    // Filter controls
    Text("Filters:");
    SameLine();
    Checkbox("Info", &m_showInfo);
    SameLine();
    Checkbox("Warning", &m_showWarning);
    SameLine();
    Checkbox("Error", &m_showError);
    SameLine();
    if (Button("Clear")) {
        clear();
    }
    SameLine();
    Checkbox("Auto-scroll", &m_autoScroll);
    
    Separator();
    
    // Log display area
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    char countText[64];
    snprintf(countText, sizeof(countText), "Logs: %zu / %d", m_logs.size(), MAX_LOGS);
    Text(countText);
    
    Separator();
    
    // Display logs
    for (const auto& entry : m_logs) {
        // Apply filters
        if (entry.level == LogEntry::Level::Info && !m_showInfo) continue;
        if (entry.level == LogEntry::Level::Warning && !m_showWarning) continue;
        if (entry.level == LogEntry::Level::Error && !m_showError) continue;
        
        // Color based on level
        Color color = Color::White();
        const char* prefix = "[INFO] ";
        
        if (entry.level == LogEntry::Level::Warning) {
            color = Color::Yellow();
            prefix = "[WARN] ";
        } else if (entry.level == LogEntry::Level::Error) {
            color = Color::Red();
            prefix = "[ERROR] ";
        }
        
        // Format: [TIME] [LEVEL] message
        char logText[512];
        snprintf(logText, sizeof(logText), "[%.2f] %s%s", 
                entry.timestamp, prefix, entry.message.c_str());
        
        TextColored(color, logText);
    }
    
    // TODO: Auto-scroll to bottom if enabled
    // This would require scroll API in UIContext
    
    ctx->endWindow();
}

void ConsoleWindow::addLog(const std::string& message, LogEntry::Level level) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    m_currentTime += 0.016f; // Approximate time increment
    m_logs.emplace_back(message, level, m_currentTime);
    
    // Limit log size
    if (m_logs.size() > MAX_LOGS) {
        m_logs.erase(m_logs.begin());
    }
}

void ConsoleWindow::clear() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logs.clear();
    addLog("Console cleared", LogEntry::Level::Info);
}

} // namespace EngineUI
