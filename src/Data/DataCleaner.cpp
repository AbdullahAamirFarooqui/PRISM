#include "Data/DataCleaner.h"
#include "Data/DataValidator.h"
#include "Algorithms/Statistics.h"
#include <cctype>

int DataCleaner::removeDuplicates(Dataset& data) {
    DynamicArray<int> duplicates = DataValidator::findDuplicateRows(data);
    if (duplicates.isEmpty())
        return 0;
    DynamicArray<bool> keep;
    keep.resize(data.rowCount(), true);
    for (int i = 0; i < duplicates.size(); ++i)
        keep[duplicates[i]] = false;
    data.removeRows(keep);
    return duplicates.size();
}

std::string DataCleaner::replacementValue(const Dataset& data, int col, MissingStrategy strategy) {
    if (!data.isNumeric(col)) {
        // Text column: use the most common value
        DynamicArray<ValueCount> top = Statistics::valueCounts(data, col, 1);
        return top.isEmpty() ? std::string("Unknown") : top[0].value;
    }
    DynamicArray<double> values = data.numericValues(col);
    if (values.isEmpty() || strategy == MissingStrategy::Zero)
        return "0";
    double replacement = (strategy == MissingStrategy::Mean) ? Statistics::mean(values)
                                                            : Statistics::median(values);
    return Dataset::formatNumber(replacement, 4);
}

// Replaces missing (or only invalid) cells of one column.
int DataCleaner::fillColumn(Dataset& data, int col, MissingStrategy strategy, bool invalidOnly) {
    std::string value = replacementValue(data, col, strategy);
    int changed = 0;
    for (int r = 0; r < data.rowCount(); ++r) {
        bool target = invalidOnly ? data.isInvalid(r, col) : data.isMissing(r, col);
        if (target) {
            data.setCell(r, col, value);
            ++changed;
        }
    }
    return changed;
}

int DataCleaner::handleMissing(Dataset& data, int col, MissingStrategy strategy) {
    int first = (col < 0) ? 0 : col;
    int last = (col < 0) ? data.columnCount() - 1 : col;

    if (strategy == MissingStrategy::RemoveRows) {
        DynamicArray<bool> keep;
        keep.resize(data.rowCount(), true);
        int removed = 0;
        for (int r = 0; r < data.rowCount(); ++r) {
            for (int c = first; c <= last; ++c) {
                if (data.isMissing(r, c)) {
                    keep[r] = false;
                    ++removed;
                    break;
                }
            }
        }
        data.removeRows(keep);
        return removed;
    }

    int changed = 0;
    for (int c = first; c <= last; ++c)
        changed += fillColumn(data, c, strategy, false);
    return changed;
}

bool DataCleaner::tryRepairNumber(const std::string& text, std::string& repaired) {
    // Keep only digits, one '.', and a leading '-'. Everything else is dropped:
    // "1,250" -> "1250"   "23.5 C" -> "23.5"   "$45" -> "45"
    std::string cleaned;
    bool seenDot = false;
    for (char ch : text) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            cleaned += ch;
        } else if (ch == '.' && !seenDot) {
            cleaned += ch;
            seenDot = true;
        } else if (ch == '-' && cleaned.empty()) {
            cleaned += ch;
        }
    }
    double ignored;
    if (!Dataset::parseNumber(cleaned, ignored))
        return false;
    repaired = cleaned;
    return true;
}

int DataCleaner::fixInvalid(Dataset& data, int col, MissingStrategy strategy) {
    int first = (col < 0) ? 0 : col;
    int last = (col < 0) ? data.columnCount() - 1 : col;
    int changed = 0;

    // Step 1: repair what can be repaired, blank out the rest (-> missing)
    for (int c = first; c <= last; ++c) {
        if (!data.isNumeric(c))
            continue;
        for (int r = 0; r < data.rowCount(); ++r) {
            if (!data.isInvalid(r, c))
                continue;
            std::string repaired;
            if (tryRepairNumber(data.cell(r, c), repaired))
                data.setCell(r, c, repaired);
            else
                data.setCell(r, c, "");
            ++changed;
        }
    }

    // Step 2: blanks created above are handled with the chosen strategy
    for (int c = first; c <= last; ++c) {
        if (!data.isNumeric(c))
            continue;
        if (strategy == MissingStrategy::RemoveRows)
            handleMissing(data, c, strategy);
        else
            fillColumn(data, c, strategy, false);
    }
    return changed;
}

int DataCleaner::trimWhitespace(Dataset& data) {
    int changed = 0;
    for (int c = 0; c < data.columnCount(); ++c)
        for (int r = 0; r < data.rowCount(); ++r) {
            std::string trimmed = Dataset::trim(data.cell(r, c));
            if (trimmed != data.cell(r, c)) {
                data.setCell(r, c, trimmed);
                ++changed;
            }
        }
    return changed;
}

bool DataCleaner::renameColumn(Dataset& data, int col, const std::string& newName) {
    std::string name = Dataset::trim(newName);
    if (col < 0 || col >= data.columnCount() || name.empty())
        return false;
    data.renameColumn(col, name);
    return true;
}

bool DataCleaner::removeColumn(Dataset& data, int col) {
    if (col < 0 || col >= data.columnCount())
        return false;
    data.removeColumn(col);
    return true;
}

std::string DataCleaner::strategyName(MissingStrategy strategy) {
    switch (strategy) {
    case MissingStrategy::RemoveRows: return "remove rows";
    case MissingStrategy::Mean:       return "mean";
    case MissingStrategy::Median:     return "median";
    case MissingStrategy::Zero:       return "zero";
    }
    return "?";
}
