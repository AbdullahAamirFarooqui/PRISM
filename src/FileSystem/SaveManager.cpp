#include "FileSystem/SaveManager.h"
#include "FileSystem/ProjectFile.h"
#include <fstream>

bool SaveManager::saveProject(const std::string& path, const Dataset& data,
                              const HistoryLog& history, std::string& error) {
    std::ofstream file(path);
    if (!file.is_open()) {
        error = "Could not write file: " + path;
        return false;
    }

    file << ProjectFile::MAGIC << '\t' << ProjectFile::VERSION << '\n';
    file << "NAME\t" << ProjectFile::escape(data.name()) << '\n';
    file << "SOURCE\t" << ProjectFile::escape(data.sourcePath()) << '\n';

    file << "COLUMNS\t" << data.columnCount() << '\n';
    for (int c = 0; c < data.columnCount(); ++c)
        file << "COLUMN\t" << (data.isNumeric(c) ? 'N' : 'T') << '\t'
             << ProjectFile::escape(data.columnName(c)) << '\n';

    file << "ROWS\t" << data.rowCount() << '\n';
    for (int r = 0; r < data.rowCount(); ++r) {
        for (int c = 0; c < data.columnCount(); ++c) {
            if (c > 0) file << '\t';
            file << ProjectFile::escape(data.cell(r, c));
        }
        file << '\n';
    }

    file << "HISTORY\t" << history.size() << '\n';
    for (const LinkedList<HistoryEntry>::Node* n = history.entries().first(); n != nullptr; n = n->next)
        file << ProjectFile::escape(n->data.timestamp) << '\t'
             << ProjectFile::escape(n->data.action) << '\n';

    file << "END\n";
    if (!file.good()) {
        error = "Error while writing " + path;
        return false;
    }
    return true;
}

std::string SaveManager::csvField(const std::string& text) {
    bool needsQuotes = text.find_first_of(",\"\n\r") != std::string::npos;
    if (!needsQuotes)
        return text;
    std::string out = "\"";
    for (char ch : text) {
        if (ch == '"') out += "\"\"";     // a quote inside quotes is doubled
        else out += ch;
    }
    out += '"';
    return out;
}

bool SaveManager::exportCSV(const std::string& path, const Dataset& data, std::string& error) {
    std::ofstream file(path);
    if (!file.is_open()) {
        error = "Could not write file: " + path;
        return false;
    }
    for (int c = 0; c < data.columnCount(); ++c) {
        if (c > 0) file << ',';
        file << csvField(data.columnName(c));
    }
    file << '\n';
    for (int r = 0; r < data.rowCount(); ++r) {
        for (int c = 0; c < data.columnCount(); ++c) {
            if (c > 0) file << ',';
            file << csvField(data.cell(r, c));
        }
        file << '\n';
    }
    return file.good();
}
