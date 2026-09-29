#ifndef PRISM_DATA_VALIDATOR_H
#define PRISM_DATA_VALIDATOR_H

#include "Data/Dataset.h"

// ---------------------------------------------------------------------------
// DataValidator  -  inspects a dataset and reports problems.
//
//  - inferTypes : a column is Numeric if at least 70% of its non-missing
//                 cells are numbers, otherwise Text.
//  - validate   : counts missing / invalid cells per column and finds
//                 duplicate rows with the HashTable (O(n) on average).
// ---------------------------------------------------------------------------

struct ColumnReport {
    std::string name;
    ColumnType type = ColumnType::Text;
    int missing = 0;
    int invalid = 0;
    int valid = 0;
};

struct ValidationReport {
    int rows = 0;
    int columns = 0;
    int numericColumns = 0;
    int totalMissing = 0;
    int totalInvalid = 0;
    int duplicateRows = 0;
    DynamicArray<ColumnReport> columnReports;
    DynamicArray<int> duplicateRowIndices;   // rows that repeat an earlier row
};

class DataValidator {
public:
    static void inferTypes(Dataset& data);
    static ValidationReport validate(const Dataset& data);
    static DynamicArray<int> findDuplicateRows(const Dataset& data);

    static constexpr double NUMERIC_THRESHOLD = 0.70;
};

#endif // PRISM_DATA_VALIDATOR_H
