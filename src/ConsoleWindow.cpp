#include "ConsoleWindow.h"
#include <ctime>
#include <iomanip>
#include <sstream>

// Static member initialization
std::vector<LogEntry> ConsoleWindow::logs;

void ConsoleWindow::update(EntityManager& em, float deltaTime) {
    auto& settings = GlobalSettings::getInstance();
    
    // Only show if the window is visible
    if (!settings.windowVisibility.showConsoleWindow) {
        return;
    }
    
    ImGui::SetNextWindowPos(ImVec2(10, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("Console", &settings.windowVisibility.showConsoleWindow)) {
        ImGui::End();
        return;
    }
    
    // Top controls
    ImGui::Checkbox("Info", &showInfo); ImGui::SameLine();
    ImGui::Checkbox("Warning", &showWarning); ImGui::SameLine();
    ImGui::Checkbox("Error", &showError); ImGui::SameLine();
    ImGui::Checkbox("Debug", &showDebug); ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll);
    
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        Clear();
    }
    
    ImGui::Separator();
    
    // Log display area
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    int displayedIndex = 0;
    for (size_t i = 0; i < logs.size(); i++) {
        const auto& entry = logs[i];
        
        // Filter by level
        bool shouldShow = false;
        switch (entry.level) {
            case LogLevel::Info: shouldShow = showInfo; break;
            case LogLevel::Warning: shouldShow = showWarning; break;
            case LogLevel::Error: shouldShow = showError; break;
            case LogLevel::Debug: shouldShow = showDebug; break;
        }
        
        if (shouldShow) {
            drawLogEntry(entry, displayedIndex++);
        }
    }
    
    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
    
    // Command input (placeholder for future command system)
    ImGui::Separator();
    if (ImGui::InputText("Command", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (inputBuffer[0]) {
            Log(std::string("Command: ") + inputBuffer, LogLevel::Debug);
            inputBuffer[0] = '\0';
        }
    }
    
    ImGui::End();
}

void ConsoleWindow::Log(const std::string& message, LogLevel level) {
    logs.emplace_back(message, level);
    
    // Limit log size
    while (logs.size() > maxLogs) {
        logs.erase(logs.begin());
    }
}

void ConsoleWindow::Clear() {
    logs.clear();
}

void ConsoleWindow::drawLogEntry(const LogEntry& entry, int index) {
    ImVec4 color = getLevelColor(entry.level);
    const char* levelText = getLevelText(entry.level);
    
    // Format timestamp
    auto timeT = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    std::string timeStr = ss.str();
    
    // Draw log line
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("[%s] [%s] %s", timeStr.c_str(), levelText, entry.message.c_str());
    ImGui::PopStyleColor();
}

ImVec4 ConsoleWindow::getLevelColor(LogLevel level) {
    switch (level) {
        case LogLevel::Info:    return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
        case LogLevel::Warning: return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
        case LogLevel::Error:   return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        case LogLevel::Debug:   return ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
        default:                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

const char* ConsoleWindow::getLevelText(LogLevel level) {
    switch (level) {
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Debug:   return "DEBUG";
        default:                return "?????";
    }
}
