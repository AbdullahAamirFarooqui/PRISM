#ifndef PRISM_DATA_CLEANER_H
#define PRISM_DATA_CLEANER_H

#include "Data/Dataset.h"

// ---------------------------------------------------------------------------
// DataCleaner - fixes the problems found by DataValidator.
//
// Every function returns how many cells / rows it changed.
// Undo is NOT handled here: the Workspace saves a snapshot before calling us.
//
// Missing-value strategies:
//   RemoveRows - delete every row that has a missing value in the column(s)
//   Mean       - replace with the column mean
//   Median     - replace with the column median
//   Zero       - replace with 0
// For TEXT columns Mean/Median/Zero all fill in the most frequent value.
// ---------------------------------------------------------------------------

enum class MissingStrategy { RemoveRows, Mean, Median, Zero };

class DataCleaner {
public:
    static int removeDuplicates(Dataset& data);

    // col = -1 means "all columns"
    static int handleMissing(Dataset& data, int col, MissingStrategy strategy);

    // Tries to repair invalid numbers first ("1,200" -> 1200, "23.5C" -> 23.5).
    // Whatever cannot be repaired is treated like a missing value.
    static int fixInvalid(Dataset& data, int col, MissingStrategy strategy);

    static int trimWhitespace(Dataset& data);
    static bool renameColumn(Dataset& data, int col, const std::string& newName);
    static bool removeColumn(Dataset& data, int col);

    // Attempts the repair described above. Returns true on success.
    static bool tryRepairNumber(const std::string& text, std::string& repaired);

    static std::string strategyName(MissingStrategy strategy);

private:
    static int fillColumn(Dataset& data, int col, MissingStrategy strategy, bool invalidOnly);
    static std::string replacementValue(const Dataset& data, int col, MissingStrategy strategy);
};

#endif // PRISM_DATA_CLEANER_H
