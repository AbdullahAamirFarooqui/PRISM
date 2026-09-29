// ---------------------------------------------------------------------------
// prism_console - uses the PRISM backend WITHOUT the GUI.
//
// Usage:  prism_console <file.csv>
// Prints the data quality report, statistics, correlations and Top-5 values,
// then writes <file>_report.html next to the CSV.
// ---------------------------------------------------------------------------
#include "Data/Workspace.h"
#include "Data/DataValidator.h"
#include "Algorithms/Statistics.h"
#include "Algorithms/Correlation.h"
#include "Algorithms/TopK.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: prism_console <file.csv>\n";
        return 1;
    }

    Workspace workspace;
    std::string error;
    if (!workspace.openCSV(argv[1], error)) {
        std::cout << "Error: " << error << "\n";
        return 1;
    }
    const Dataset& data = workspace.data();

    ValidationReport report = DataValidator::validate(data);
    std::cout << "Dataset '" << data.name() << "': " << report.rows << " rows, "
              << report.columns << " columns\n"
              << "Missing: " << report.totalMissing << "  Invalid: " << report.totalInvalid
              << "  Duplicates: " << report.duplicateRows << "\n\n";

    std::cout << "Column statistics\n-----------------\n";
    DynamicArray<int> numeric = data.numericColumnIndices();
    for (int i = 0; i < numeric.size(); ++i) {
        ColumnSummary s = Statistics::summarize(data.numericValues(numeric[i]));
        std::cout << data.columnName(numeric[i]) << ": mean=" << Dataset::formatNumber(s.mean, 2)
                  << " median=" << Dataset::formatNumber(s.median, 2)
                  << " std=" << Dataset::formatNumber(s.stdDev, 2)
                  << " min=" << Dataset::formatNumber(s.min, 2)
                  << " max=" << Dataset::formatNumber(s.max, 2) << "\n";
    }

    std::cout << "\nStrong correlations (|r| >= 0.7)\n--------------------------------\n";
    CorrelationMatrix matrix = Correlation::computeMatrix(data);
    DynamicArray<StrongPair> pairs = Correlation::strongPairs(matrix, 0.7);
    for (int i = 0; i < pairs.size(); ++i)
        std::cout << pairs[i].first << " <-> " << pairs[i].second << "  r = "
                  << Dataset::formatNumber(pairs[i].r, 3) << "\n";
    if (pairs.isEmpty())
        std::cout << "(none)\n";

    if (!numeric.isEmpty()) {
        int col = numeric[numeric.size() - 1];
        std::cout << "\nTop 5 of " << data.columnName(col) << ":";
        DynamicArray<RankedItem> top = TopK::largest(data, col, 5);
        for (int i = 0; i < top.size(); ++i)
            std::cout << " " << Dataset::formatNumber(top[i].value, 2);
        std::cout << "\n";
    }

    std::string reportPath = std::string(argv[1]) + "_report.html";
    if (workspace.exportReport(reportPath, error))
        std::cout << "\nReport written to " << reportPath << "\n";
    return 0;
}
