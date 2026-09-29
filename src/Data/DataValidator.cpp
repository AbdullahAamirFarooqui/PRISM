#include "Data/DataValidator.h"
#include "DSA/HashTable.h"

void DataValidator::inferTypes(Dataset& data) {
    for (int c = 0; c < data.columnCount(); ++c) {
        int filled = 0;
        int numbers = 0;
        double ignored;
        for (int r = 0; r < data.rowCount(); ++r) {
            if (data.isMissing(r, c))
                continue;
            ++filled;
            if (Dataset::parseNumber(data.cell(r, c), ignored))
                ++numbers;
        }
        bool numeric = filled > 0 && numbers >= NUMERIC_THRESHOLD * filled;
        data.setColumnType(c, numeric ? ColumnType::Numeric : ColumnType::Text);
    }
}

DynamicArray<int> DataValidator::findDuplicateRows(const Dataset& data) {
    DynamicArray<int> duplicates;
    HashTable seen(data.rowCount() * 2 + 11);
    for (int r = 0; r < data.rowCount(); ++r) {
        std::string signature = data.rowSignature(r);
        if (seen.contains(signature))
            duplicates.pushBack(r);        // same row was seen before
        seen.insert(signature, r);
    }
    return duplicates;
}

ValidationReport DataValidator::validate(const Dataset& data) {
    ValidationReport report;
    report.rows = data.rowCount();
    report.columns = data.columnCount();

    for (int c = 0; c < data.columnCount(); ++c) {
        ColumnReport col;
        col.name = data.columnName(c);
        col.type = data.columnType(c);
        for (int r = 0; r < data.rowCount(); ++r) {
            if (data.isMissing(r, c)) ++col.missing;
            else if (data.isInvalid(r, c)) ++col.invalid;
            else ++col.valid;
        }
        if (col.type == ColumnType::Numeric)
            ++report.numericColumns;
        report.totalMissing += col.missing;
        report.totalInvalid += col.invalid;
        report.columnReports.pushBack(col);
    }

    report.duplicateRowIndices = findDuplicateRows(data);
    report.duplicateRows = report.duplicateRowIndices.size();
    return report;
}
