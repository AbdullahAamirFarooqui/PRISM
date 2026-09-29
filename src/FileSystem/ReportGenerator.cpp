#include "FileSystem/ReportGenerator.h"
#include "Data/DataValidator.h"
#include "Algorithms/Statistics.h"
#include "Algorithms/Correlation.h"
#include "Algorithms/TopK.h"
#include "DSA/Graph.h"
#include <fstream>
#include <cmath>
#include <cstdio>

std::string ReportGenerator::escapeHtml(const std::string& text) {
    std::string out;
    for (char ch : text) {
        if (ch == '<') out += "&lt;";
        else if (ch == '>') out += "&gt;";
        else if (ch == '&') out += "&amp;";
        else if (ch == '"') out += "&quot;";
        else out += ch;
    }
    return out;
}

// Blue for positive, red for negative; stronger = more intense.
std::string ReportGenerator::correlationColor(double r) {
    if (std::isnan(r))
        return "#eeeeee";
    int intensity = static_cast<int>(std::fabs(r) * 155);
    char buffer[16];
    if (r >= 0)
        std::snprintf(buffer, sizeof(buffer), "#%02x%02xff", 255 - intensity, 255 - intensity / 2);
    else
        std::snprintf(buffer, sizeof(buffer), "#ff%02x%02x", 255 - intensity, 255 - intensity);
    return buffer;
}

static std::string num(double v) { return Dataset::formatNumber(v, 3); }

std::string ReportGenerator::buildHtml(const Dataset& data, const HistoryLog& history) {
    std::string h;
    h += "<!DOCTYPE html><html><head><meta charset='utf-8'><title>PRISM Report - " +
         escapeHtml(data.name()) + "</title><style>"
         "body{font-family:Segoe UI,Arial,sans-serif;margin:32px;color:#222;max-width:1100px}"
         "h1{color:#3b2c85}h2{color:#3b2c85;border-bottom:2px solid #ddd;padding-bottom:4px;margin-top:32px}"
         "table{border-collapse:collapse;margin:8px 0}td,th{border:1px solid #ccc;padding:4px 10px;text-align:right}"
         "th{background:#f0eefa}td.l,th.l{text-align:left}.card{display:inline-block;background:#f6f5fc;"
         "border-radius:8px;padding:10px 18px;margin:4px;min-width:110px}.card b{font-size:22px;display:block}"
         "</style></head><body>";

    h += "<h1>PRISM &mdash; Scientific Data Analysis Report</h1>";
    h += "<p>Dataset: <b>" + escapeHtml(data.name()) + "</b><br>Source: " + escapeHtml(data.sourcePath()) +
         "<br>Generated: " + HistoryLog::currentTimestamp() + "</p>";

    // ----- 1. overview -----
    ValidationReport report = DataValidator::validate(data);
    h += "<h2>1. Overview &amp; Data Quality</h2>";
    h += "<div class='card'><b>" + std::to_string(report.rows) + "</b>rows</div>";
    h += "<div class='card'><b>" + std::to_string(report.columns) + "</b>columns</div>";
    h += "<div class='card'><b>" + std::to_string(report.numericColumns) + "</b>numeric</div>";
    h += "<div class='card'><b>" + std::to_string(report.totalMissing) + "</b>missing cells</div>";
    h += "<div class='card'><b>" + std::to_string(report.totalInvalid) + "</b>invalid cells</div>";
    h += "<div class='card'><b>" + std::to_string(report.duplicateRows) + "</b>duplicate rows</div>";

    h += "<table><tr><th class='l'>Column</th><th class='l'>Type</th><th>Valid</th><th>Missing</th><th>Invalid</th></tr>";
    for (int i = 0; i < report.columnReports.size(); ++i) {
        const ColumnReport& c = report.columnReports[i];
        h += "<tr><td class='l'>" + escapeHtml(c.name) + "</td><td class='l'>" +
             (c.type == ColumnType::Numeric ? "Numeric" : "Text") + "</td><td>" +
             std::to_string(c.valid) + "</td><td>" + std::to_string(c.missing) + "</td><td>" +
             std::to_string(c.invalid) + "</td></tr>";
    }
    h += "</table>";

    // ----- 2. descriptive statistics -----
    h += "<h2>2. Descriptive Statistics</h2><table><tr><th class='l'>Column</th><th>Count</th><th>Mean</th>"
         "<th>Median</th><th>Mode</th><th>Std Dev</th><th>Variance</th><th>Min</th><th>Q1</th><th>Q3</th>"
         "<th>Max</th><th>Range</th></tr>";
    DynamicArray<int> numeric = data.numericColumnIndices();
    for (int i = 0; i < numeric.size(); ++i) {
        ColumnSummary s = Statistics::summarize(data.numericValues(numeric[i]));
        h += "<tr><td class='l'>" + escapeHtml(data.columnName(numeric[i])) + "</td><td>" +
             std::to_string(s.count) + "</td><td>" + num(s.mean) + "</td><td>" + num(s.median) +
             "</td><td>" + num(s.mode) + "</td><td>" + num(s.stdDev) + "</td><td>" + num(s.variance) +
             "</td><td>" + num(s.min) + "</td><td>" + num(s.q1) + "</td><td>" + num(s.q3) + "</td><td>" +
             num(s.max) + "</td><td>" + num(s.range) + "</td></tr>";
    }
    h += "</table>";

    // text columns: most frequent values
    for (int c = 0; c < data.columnCount(); ++c) {
        if (data.isNumeric(c))
            continue;
        DynamicArray<ValueCount> top = Statistics::valueCounts(data, c, 5);
        h += "<p><b>" + escapeHtml(data.columnName(c)) + "</b> (text) &mdash; most frequent: ";
        for (int i = 0; i < top.size(); ++i)
            h += escapeHtml(top[i].value) + " (" + std::to_string(top[i].count) + ")" +
                 (i + 1 < top.size() ? ", " : "");
        h += "</p>";
    }

    // ----- 3. Top-K -----
    if (!numeric.isEmpty()) {
        h += "<h2>3. Top-5 Rankings (heap based)</h2><table><tr><th class='l'>Column</th><th class='l'>Largest values</th></tr>";
        for (int i = 0; i < numeric.size(); ++i) {
            DynamicArray<RankedItem> top = TopK::largest(data, numeric[i], 5);
            std::string list;
            for (int k = 0; k < top.size(); ++k)
                list += num(top[k].value) + " (row " + std::to_string(top[k].row + 1) + ")" +
                        (k + 1 < top.size() ? ", " : "");
            h += "<tr><td class='l'>" + escapeHtml(data.columnName(numeric[i])) + "</td><td class='l'>" +
                 list + "</td></tr>";
        }
        h += "</table>";
    }

    // ----- 4. correlation -----
    CorrelationMatrix matrix = Correlation::computeMatrix(data);
    if (matrix.size() >= 2) {
        h += "<h2>4. Pearson Correlation Matrix</h2><table><tr><th></th>";
        for (int j = 0; j < matrix.size(); ++j)
            h += "<th>" + escapeHtml(matrix.names[j]) + "</th>";
        h += "</tr>";
        for (int i = 0; i < matrix.size(); ++i) {
            h += "<tr><th class='l'>" + escapeHtml(matrix.names[i]) + "</th>";
            for (int j = 0; j < matrix.size(); ++j)
                h += "<td style='background:" + correlationColor(matrix.r[i][j]) + "'>" +
                     num(matrix.r[i][j]) + "</td>";
            h += "</tr>";
        }
        h += "</table>";

        double threshold = Correlation::DEFAULT_THRESHOLD;
        DynamicArray<StrongPair> pairs = Correlation::strongPairs(matrix, threshold);
        h += "<h2>5. Relationship Network (|r| &ge; " + num(threshold) + ")</h2>";
        if (pairs.isEmpty()) {
            h += "<p>No strongly correlated column pairs were found.</p>";
        } else {
            h += "<table><tr><th class='l'>Column A</th><th class='l'>Column B</th><th>r</th><th class='l'>Meaning</th></tr>";
            for (int i = 0; i < pairs.size(); ++i)
                h += "<tr><td class='l'>" + escapeHtml(pairs[i].first) + "</td><td class='l'>" +
                     escapeHtml(pairs[i].second) + "</td><td>" + num(pairs[i].r) + "</td><td class='l'>" +
                     Correlation::describe(pairs[i].r) + "</td></tr>";
            h += "</table>";
        }

        Graph graph;
        Correlation::buildGraph(matrix, threshold, graph);
        DynamicArray<int> componentOf;
        int clusters = graph.connectedComponents(componentOf);
        h += "<p>Clusters found with DFS: <b>" + std::to_string(clusters) + "</b></p><ul>";
        for (int c = 0; c < clusters; ++c) {
            std::string members;
            int count = 0;
            for (int n = 0; n < graph.nodeCount(); ++n)
                if (componentOf[n] == c) {
                    members += (count > 0 ? ", " : "") + escapeHtml(graph.label(n));
                    ++count;
                }
            h += "<li>" + members + (count == 1 ? " <i>(isolated)</i>" : "") + "</li>";
        }
        h += "</ul>";
    }

    // ----- 6. history -----
    h += "<h2>6. Operation History</h2><table><tr><th class='l'>Time</th><th class='l'>Action</th></tr>";
    for (const LinkedList<HistoryEntry>::Node* n = history.entries().first(); n != nullptr; n = n->next)
        h += "<tr><td class='l'>" + escapeHtml(n->data.timestamp) + "</td><td class='l'>" +
             escapeHtml(n->data.action) + "</td></tr>";
    h += "</table><p style='color:#888;margin-top:40px'>Generated by PRISM &mdash; Scientific Data Analyzer</p>";
    h += "</body></html>";
    return h;
}

bool ReportGenerator::writeHtml(const std::string& path, const Dataset& data,
                                const HistoryLog& history, std::string& error) {
    std::ofstream file(path);
    if (!file.is_open()) {
        error = "Could not write report: " + path;
        return false;
    }
    file << buildHtml(data, history);
    return file.good();
}
