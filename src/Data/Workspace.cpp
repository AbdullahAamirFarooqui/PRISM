#include "Data/Workspace.h"
#include "Data/CSVReader.h"
#include "Data/DataValidator.h"
#include "FileSystem/SaveManager.h"
#include "FileSystem/LoadManager.h"
#include "FileSystem/ReportGenerator.h"

bool Workspace::openCSV(const std::string& path, std::string& error) {
    Dataset loaded;
    if (!CSVReader::read(path, loaded, error))
        return false;

    m_data = loaded;
    m_undo.clear();
    m_history.clear();
    m_projectPath.clear();
    ++m_version;

    ValidationReport report = DataValidator::validate(m_data);
    std::string text = "Imported '" + m_data.name() + "' (" + std::to_string(m_data.rowCount()) +
                       " rows, " + std::to_string(m_data.columnCount()) + " columns, " +
                       std::to_string(report.totalMissing) + " missing, " +
                       std::to_string(report.totalInvalid) + " invalid, " +
                       std::to_string(report.duplicateRows) + " duplicates)";
    if (CSVReader::lastMalformedRows() > 0)
        text += " - " + std::to_string(CSVReader::lastMalformedRows()) + " malformed rows";
    m_history.add(text);
    return true;
}

bool Workspace::openComparisonCSV(const std::string& path, std::string& error) {
    Dataset loaded;
    if (!CSVReader::read(path, loaded, error))
        return false;
    m_comparison = loaded;
    m_history.add("Loaded comparison dataset '" + m_comparison.name() + "'");
    ++m_version;
    return true;
}

void Workspace::closeComparison() {
    m_comparison.clear();
    ++m_version;
}

bool Workspace::saveProject(const std::string& path, std::string& error) {
    if (!SaveManager::saveProject(path, m_data, m_history, error))
        return false;
    m_projectPath = path;
    m_history.add("Saved project to " + path);
    return true;
}

bool Workspace::openProject(const std::string& path, std::string& error) {
    Dataset loaded;
    HistoryLog loadedHistory;
    if (!LoadManager::loadProject(path, loaded, loadedHistory, error))
        return false;
    m_data = loaded;
    m_history = loadedHistory;
    m_undo.clear();
    m_projectPath = path;
    m_history.add("Opened project " + path);
    ++m_version;
    return true;
}

bool Workspace::exportCSV(const std::string& path, std::string& error) {
    if (!SaveManager::exportCSV(path, m_data, error))
        return false;
    m_history.add("Exported cleaned data to " + path);
    return true;
}

bool Workspace::exportReport(const std::string& path, std::string& error) {
    m_history.add("Generated analysis report " + path);
    return ReportGenerator::writeHtml(path, m_data, m_history, error);
}

void Workspace::beginChange(const std::string& description) {
    m_undo.saveState(m_data, description);
}

void Workspace::endChange(const std::string& historyText) {
    m_history.add(historyText);
    ++m_version;
}

bool Workspace::undo(std::string& description) {
    if (!m_undo.undo(m_data, description))
        return false;
    m_history.add("Undo: " + description);
    ++m_version;
    return true;
}

bool Workspace::redo(std::string& description) {
    if (!m_undo.redo(m_data, description))
        return false;
    m_history.add("Redo: " + description);
    ++m_version;
    return true;
}
