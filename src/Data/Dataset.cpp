#include "Data/Dataset.h"
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cmath>

void Dataset::clear() {
    m_columns.clear();
    m_rowCount = 0;
    m_name.clear();
    m_sourcePath.clear();
}

void Dataset::addColumn(const std::string& name, ColumnType type) {
    Column column;
    column.name = name;
    column.type = type;
    column.cells.resize(m_rowCount, "");   // existing rows get an empty cell
    m_columns.pushBack(column);
}

void Dataset::addRow(const DynamicArray<std::string>& cells) {
    for (int c = 0; c < m_columns.size(); ++c)
        m_columns[c].cells.pushBack(c < cells.size() ? cells[c] : std::string());
    ++m_rowCount;
}

bool Dataset::isMissing(int row, int col) const {
    return isMissingText(m_columns[col].cells[row]);
}

bool Dataset::isInvalid(int row, int col) const {
    if (!isNumeric(col) || isMissing(row, col))
        return false;
    double ignored;
    return !parseNumber(m_columns[col].cells[row], ignored);
}

bool Dataset::numberAt(int row, int col, double& out) const {
    return parseNumber(m_columns[col].cells[row], out);
}

int Dataset::findColumn(const std::string& name) const {
    for (int c = 0; c < m_columns.size(); ++c)
        if (m_columns[c].name == name)
            return c;
    return -1;
}

void Dataset::renameColumn(int col, const std::string& newName) {
    m_columns[col].name = newName;
}

void Dataset::removeColumn(int col) {
    m_columns.removeAt(col);
    if (m_columns.isEmpty())
        m_rowCount = 0;
}

void Dataset::removeRows(const DynamicArray<bool>& keep) {
    int newCount = 0;
    for (int c = 0; c < m_columns.size(); ++c) {
        DynamicArray<std::string>& cells = m_columns[c].cells;
        int write = 0;
        for (int r = 0; r < m_rowCount; ++r) {
            if (keep[r]) {
                cells[write] = cells[r];   // compact the kept rows to the front
                ++write;
            }
        }
        cells.resize(write);
        newCount = write;
    }
    m_rowCount = m_columns.isEmpty() ? 0 : newCount;
}

DynamicArray<double> Dataset::numericValues(int col) const {
    DynamicArray<double> values;
    values.reserve(m_rowCount);
    double number;
    for (int r = 0; r < m_rowCount; ++r)
        if (parseNumber(m_columns[col].cells[r], number))
            values.pushBack(number);
    return values;
}

void Dataset::numericValuesWithRows(int col, DynamicArray<double>& values, DynamicArray<int>& rows) const {
    values.clear();
    rows.clear();
    double number;
    for (int r = 0; r < m_rowCount; ++r) {
        if (parseNumber(m_columns[col].cells[r], number)) {
            values.pushBack(number);
            rows.pushBack(r);
        }
    }
}

DynamicArray<int> Dataset::numericColumnIndices() const {
    DynamicArray<int> indices;
    for (int c = 0; c < m_columns.size(); ++c)
        if (isNumeric(c))
            indices.pushBack(c);
    return indices;
}

std::string Dataset::rowSignature(int row) const {
    std::string signature;
    for (int c = 0; c < m_columns.size(); ++c) {
        signature += trim(m_columns[c].cells[row]);
        signature += '\x1F';   // "unit separator" character, never appears in normal data
    }
    return signature;
}

// ------------------------------ static helpers -----------------------------

bool Dataset::isMissingText(const std::string& text) {
    std::string t = trim(text);
    if (t.empty())
        return true;
    std::string lower;
    for (char ch : t)
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return lower == "na" || lower == "n/a" || lower == "nan" || lower == "null" ||
           lower == "none" || lower == "?" || lower == "-" || lower == "missing";
}

bool Dataset::parseNumber(const std::string& text, double& out) {
    std::string t = trim(text);
    if (t.empty())
        return false;
    char* end = nullptr;
    double value = std::strtod(t.c_str(), &end);
    if (end == t.c_str() || *end != '\0')   // nothing parsed, or junk after the number
        return false;
    if (std::isnan(value) || std::isinf(value))
        return false;
    out = value;
    return true;
}

std::string Dataset::formatNumber(double value, int decimals) {
    if (std::isnan(value))
        return "-";
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
    std::string s = buffer;
    // remove trailing zeros: 12.5000 -> 12.5 , 3.0000 -> 3
    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0')
            s.pop_back();
        if (!s.empty() && s.back() == '.')
            s.pop_back();
    }
    if (s == "-0")
        s = "0";
    return s;
}

std::string Dataset::trim(const std::string& text) {
    int start = 0;
    int end = static_cast<int>(text.size()) - 1;
    while (start <= end && std::isspace(static_cast<unsigned char>(text[start])))
        ++start;
    while (end >= start && std::isspace(static_cast<unsigned char>(text[end])))
        --end;
    return text.substr(start, end - start + 1);
}
