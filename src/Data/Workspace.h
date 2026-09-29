#ifndef PRISM_WORKSPACE_H
#define PRISM_WORKSPACE_H

#include "Data/Dataset.h"
#include "Data/UndoManager.h"
#include "Data/HistoryLog.h"
#include <string>

// ---------------------------------------------------------------------------
// Workspace - everything that belongs to one PRISM session:
//
//   - the active dataset (+ an optional second dataset for comparison)
//   - the undo/redo manager
//   - the history log
//
// The GUI only talks to the Workspace, so the backend can also be used
// without any GUI (see tests/ and the console demo).
//
// How to make an undoable change:
//     workspace.beginChange("Removed duplicates");   // snapshot for undo
//     DataCleaner::removeDuplicates(workspace.data());
//     workspace.endChange("Removed 12 duplicate rows");  // history entry
// ---------------------------------------------------------------------------
class Workspace {
public:
    // ----- files -----
    bool openCSV(const std::string& path, std::string& error);
    bool openComparisonCSV(const std::string& path, std::string& error);
    bool saveProject(const std::string& path, std::string& error);
    bool openProject(const std::string& path, std::string& error);
    bool exportCSV(const std::string& path, std::string& error);
    bool exportReport(const std::string& path, std::string& error);

    // ----- data -----
    Dataset& data() { return m_data; }
    const Dataset& data() const { return m_data; }
    const Dataset& comparison() const { return m_comparison; }
    bool hasData() const { return !m_data.isEmpty(); }
    bool hasComparison() const { return !m_comparison.isEmpty(); }
    void closeComparison();

    // ----- undo / redo -----
    void beginChange(const std::string& description);
    void endChange(const std::string& historyText);
    bool undo(std::string& description);
    bool redo(std::string& description);
    const UndoManager& undoManager() const { return m_undo; }

    // ----- history -----
    void log(const std::string& action) { m_history.add(action); }
    const HistoryLog& history() const { return m_history; }

    // Increases every time the data changes (views use it to know when to refresh).
    int version() const { return m_version; }
    const std::string& projectPath() const { return m_projectPath; }

private:
    Dataset m_data;
    Dataset m_comparison;
    UndoManager m_undo;
    HistoryLog m_history;
    int m_version = 0;
    std::string m_projectPath;
};

#endif // PRISM_WORKSPACE_H
