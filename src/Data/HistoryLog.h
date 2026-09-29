#ifndef PRISM_HISTORY_LOG_H
#define PRISM_HISTORY_LOG_H

#include "DSA/LinkedList.h"
#include <string>

// ---------------------------------------------------------------------------
// HistoryLog - timestamped list of every operation, stored in our doubly
// linked list. Shown in the History panel and written into the report.
// ---------------------------------------------------------------------------

struct HistoryEntry {
    std::string timestamp;   // "2026-09-29 14:05:31"
    std::string action;
};

class HistoryLog {
public:
    void add(const std::string& action);                          // uses current time
    void addEntry(const std::string& timestamp, const std::string& action);

    const LinkedList<HistoryEntry>& entries() const { return m_entries; }
    int size() const { return m_entries.size(); }
    void clear() { m_entries.clear(); }

    static std::string currentTimestamp();

private:
    LinkedList<HistoryEntry> m_entries;
};

#endif // PRISM_HISTORY_LOG_H
