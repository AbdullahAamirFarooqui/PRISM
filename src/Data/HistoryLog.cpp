#include "Data/HistoryLog.h"
#include <ctime>

void HistoryLog::add(const std::string& action) {
    addEntry(currentTimestamp(), action);
}

void HistoryLog::addEntry(const std::string& timestamp, const std::string& action) {
    HistoryEntry entry;
    entry.timestamp = timestamp;
    entry.action = action;
    m_entries.pushBack(entry);
}

std::string HistoryLog::currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local);
    return buffer;
}
