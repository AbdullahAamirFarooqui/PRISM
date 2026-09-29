#include "FileSystem/LoadManager.h"
#include "FileSystem/ProjectFile.h"
#include <fstream>
#include <cstdlib>

// Reads the next line and checks it starts with the expected keyword.
static bool readKeyLine(std::ifstream& file, const std::string& keyword,
                        DynamicArray<std::string>& fields, std::string& error) {
    std::string line;
    if (!std::getline(file, line)) {
        error = "Unexpected end of file (expected " + keyword + ")";
        return false;
    }
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    fields = ProjectFile::splitFields(line);
    if (fields[0] != keyword) {
        error = "Corrupted project file: expected '" + keyword + "' but found '" + fields[0] + "'";
        return false;
    }
    return true;
}

bool LoadManager::loadProject(const std::string& path, Dataset& data,
                              HistoryLog& history, std::string& error) {
    std::ifstream file(path);
    if (!file.is_open()) {
        error = "Could not open file: " + path;
        return false;
    }

    DynamicArray<std::string> fields;
    if (!readKeyLine(file, ProjectFile::MAGIC, fields, error)) {
        error = "This is not a PRISM project (.sda) file.";
        return false;
    }

    Dataset loaded;
    HistoryLog loadedHistory;

    if (!readKeyLine(file, "NAME", fields, error)) return false;
    loaded.setName(fields.size() > 1 ? fields[1] : "");
    if (!readKeyLine(file, "SOURCE", fields, error)) return false;
    loaded.setSourcePath(fields.size() > 1 ? fields[1] : "");

    if (!readKeyLine(file, "COLUMNS", fields, error)) return false;
    int columnCount = std::atoi(fields[1].c_str());
    for (int c = 0; c < columnCount; ++c) {
        if (!readKeyLine(file, "COLUMN", fields, error)) return false;
        if (fields.size() < 3) {
            error = "Corrupted COLUMN line";
            return false;
        }
        loaded.addColumn(fields[2], fields[1] == "N" ? ColumnType::Numeric : ColumnType::Text);
    }

    if (!readKeyLine(file, "ROWS", fields, error)) return false;
    int rowCount = std::atoi(fields[1].c_str());
    std::string line;
    for (int r = 0; r < rowCount; ++r) {
        if (!std::getline(file, line)) {
            error = "Project file ended early while reading rows";
            return false;
        }
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        loaded.addRow(ProjectFile::splitFields(line));
    }

    if (!readKeyLine(file, "HISTORY", fields, error)) return false;
    int historyCount = std::atoi(fields[1].c_str());
    for (int h = 0; h < historyCount; ++h) {
        if (!std::getline(file, line))
            break;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        DynamicArray<std::string> entry = ProjectFile::splitFields(line);
        loadedHistory.addEntry(entry[0], entry.size() > 1 ? entry[1] : "");
    }

    data = loaded;
    history = loadedHistory;
    return true;
}
