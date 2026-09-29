#ifndef PRISM_REPORT_GENERATOR_H
#define PRISM_REPORT_GENERATOR_H

#include "Data/Dataset.h"
#include "Data/HistoryLog.h"
#include <string>

// ---------------------------------------------------------------------------
// ReportGenerator - builds an HTML analysis report that can be opened in any
// web browser: overview, data quality, statistics for every column,
// correlation matrix, relationship clusters and the full operation history.
// ---------------------------------------------------------------------------
class ReportGenerator {
public:
    static std::string buildHtml(const Dataset& data, const HistoryLog& history);
    static bool writeHtml(const std::string& path, const Dataset& data,
                          const HistoryLog& history, std::string& error);

private:
    static std::string escapeHtml(const std::string& text);
    static std::string correlationColor(double r);
};

#endif // PRISM_REPORT_GENERATOR_H
