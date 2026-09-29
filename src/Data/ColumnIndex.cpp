#include "Data/ColumnIndex.h"
#include <cstdlib>

void ColumnIndex::clear() {
    m_tree.clear();
    m_hash.clear();
    m_column = -1;
}

void ColumnIndex::build(const Dataset& data, int col) {
    clear();
    if (col < 0 || col >= data.columnCount())
        return;
    m_column = col;

    // 1) Hash table: every non-missing cell, keyed by its text
    for (int r = 0; r < data.rowCount(); ++r)
        if (!data.isMissing(r, col))
            m_hash.insert(data.cell(r, col), r);

    // 2) BST: numeric columns only, inserted in shuffled order
    if (!data.isNumeric(col))
        return;
    DynamicArray<double> values;
    DynamicArray<int> rows;
    data.numericValuesWithRows(col, values, rows);

    std::srand(12345);                               // fixed seed = same tree every run
    for (int i = values.size() - 1; i > 0; --i) {    // Fisher-Yates shuffle
        int j = std::rand() % (i + 1);
        values.swap(i, j);
        rows.swap(i, j);
    }
    for (int i = 0; i < values.size(); ++i)
        m_tree.insert(values[i], rows[i]);
}

const DynamicArray<int>* ColumnIndex::findExact(const std::string& value) const {
    return m_hash.find(value);
}

DynamicArray<int> ColumnIndex::findRange(double low, double high) const {
    DynamicArray<int> rows;
    m_tree.rangeQuery(low, high, rows);
    return rows;
}
