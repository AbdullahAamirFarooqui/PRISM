#ifndef PRISM_DATASET_H
#define PRISM_DATASET_H

#include "DSA/DynamicArray.h"
#include <string>

// ---------------------------------------------------------------------------
// Dataset  -  the single shared table every module works with.
//
// Stored COLUMN by column (each Column has its own DynamicArray of cells).
// That makes statistics fast: a whole column is one contiguous array.
// Every cell is kept as text exactly as it was read; numeric columns are
// converted to double on demand with numberAt().
// ---------------------------------------------------------------------------

enum class ColumnType { Numeric, Text };

struct Column {
    std::string name;
    ColumnType type = ColumnType::Text;
    DynamicArray<std::string> cells;
};

class Dataset {
public:
    void clear();

    // ----- basic info -----
    int rowCount() const { return m_rowCount; }
    int columnCount() const { return m_columns.size(); }
    bool isEmpty() const { return m_columns.isEmpty(); }

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    const std::string& sourcePath() const { return m_sourcePath; }
    void setSourcePath(const std::string& path) { m_sourcePath = path; }

    // ----- building the table -----
    void addColumn(const std::string& name, ColumnType type = ColumnType::Text);
    void addRow(const DynamicArray<std::string>& cells);   // missing cells -> ""

    // ----- cells -----
    const std::string& cell(int row, int col) const { return m_columns[col].cells[row]; }
    void setCell(int row, int col, const std::string& value) { m_columns[col].cells[row] = value; }

    bool isMissing(int row, int col) const;
    bool isInvalid(int row, int col) const;           // non-number in a numeric column
    bool numberAt(int row, int col, double& out) const;

    // ----- columns -----
    const Column& column(int col) const { return m_columns[col]; }
    const std::string& columnName(int col) const { return m_columns[col].name; }
    ColumnType columnType(int col) const { return m_columns[col].type; }
    bool isNumeric(int col) const { return m_columns[col].type == ColumnType::Numeric; }
    void setColumnType(int col, ColumnType type) { m_columns[col].type = type; }
    int findColumn(const std::string& name) const;     // -1 if not found

    void renameColumn(int col, const std::string& newName);
    void removeColumn(int col);

    // keep[row] == false  ->  that row is deleted. One O(n*m) pass.
    void removeRows(const DynamicArray<bool>& keep);

    // All valid numbers of a column (missing / invalid cells are skipped).
    DynamicArray<double> numericValues(int col) const;
    // Same, plus the row each number came from.
    void numericValuesWithRows(int col, DynamicArray<double>& values, DynamicArray<int>& rows) const;

    DynamicArray<int> numericColumnIndices() const;

    // One string that represents a whole row (used to find duplicate rows).
    std::string rowSignature(int row) const;

    // ----- helpers usable anywhere -----
    static bool isMissingText(const std::string& text);            // "", "NA", "null", "?" ...
    static bool parseNumber(const std::string& text, double& out); // strict conversion
    static std::string formatNumber(double value, int decimals = 4);
    static std::string trim(const std::string& text);

private:
    DynamicArray<Column> m_columns;
    int m_rowCount = 0;
    std::string m_name;
    std::string m_sourcePath;
};

#endif // PRISM_DATASET_H
