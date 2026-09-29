#include "Data/CSVReader.h"
#include "Data/DataValidator.h"
#include <fstream>

int CSVReader::s_malformedRows = 0;

int CSVReader::lastMalformedRows() {
    return s_malformedRows;
}

// Picks the delimiter that appears most often in the header line.
char CSVReader::detectDelimiter(const std::string& headerLine) {
    const char candidates[3] = {',', ';', '\t'};
    char best = ',';
    int bestCount = 0;
    for (char candidate : candidates) {
        int count = 0;
        for (char ch : headerLine)
            if (ch == candidate)
                ++count;
        if (count > bestCount) {
            bestCount = count;
            best = candidate;
        }
    }
    return best;
}

// Splits one line into fields. Handles quotes:  a,"b, with comma",c
// and escaped quotes inside quotes:  "He said ""hi"""
DynamicArray<std::string> CSVReader::splitLine(const std::string& line, char delimiter) {
    DynamicArray<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {   // "" -> "
                    current += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                current += ch;
            }
        } else if (ch == '"') {
            inQuotes = true;
        } else if (ch == delimiter) {
            fields.pushBack(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    fields.pushBack(current);
    return fields;
}

bool CSVReader::read(const std::string& path, Dataset& out, std::string& error) {
    s_malformedRows = 0;
    std::ifstream file(path);
    if (!file.is_open()) {
        error = "Could not open file: " + path;
        return false;
    }

    std::string line;
    if (!std::getline(file, line)) {
        error = "The file is empty.";
        return false;
    }

    // Remove a UTF-8 BOM (invisible bytes Excel sometimes adds) and '\r'
    if (line.size() >= 3 && (unsigned char)line[0] == 0xEF &&
        (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
        line = line.substr(3);
    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    Dataset loaded;
    char delimiter = detectDelimiter(line);
    DynamicArray<std::string> header = splitLine(line, delimiter);
    for (int c = 0; c < header.size(); ++c) {
        std::string name = Dataset::trim(header[c]);
        if (name.empty())
            name = "Column " + std::to_string(c + 1);
        loaded.addColumn(name);
    }

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (Dataset::trim(line).empty())
            continue;                          // skip blank lines
        DynamicArray<std::string> cells = splitLine(line, delimiter);
        if (cells.size() != loaded.columnCount())
            ++s_malformedRows;
        for (int c = 0; c < cells.size(); ++c)
            cells[c] = Dataset::trim(cells[c]);
        loaded.addRow(cells);
    }

    if (loaded.columnCount() == 0) {
        error = "No columns found in the header line.";
        return false;
    }

    // Name the dataset after the file (without folders and extension).
    std::string fileName = path;
    size_t slash = fileName.find_last_of("/\\");
    if (slash != std::string::npos)
        fileName = fileName.substr(slash + 1);
    size_t dot = fileName.find_last_of('.');
    if (dot != std::string::npos)
        fileName = fileName.substr(0, dot);

    loaded.setName(fileName);
    loaded.setSourcePath(path);
    DataValidator::inferTypes(loaded);

    out = loaded;
    return true;
}
