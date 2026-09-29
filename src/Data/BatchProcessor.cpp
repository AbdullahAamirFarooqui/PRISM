#include "Data/BatchProcessor.h"
#include "Data/DataValidator.h"
#include "Algorithms/Statistics.h"
#include "Algorithms/Correlation.h"
#include "Algorithms/TopK.h"
#include "DSA/Graph.h"

// ------------------------------- jobs -------------------------------------

std::string ValidationJob::run(const Dataset& data) {
    ValidationReport report = DataValidator::validate(data);
    return std::to_string(report.rows) + " rows, " + std::to_string(report.columns) + " columns, " +
           std::to_string(report.totalMissing) + " missing, " +
           std::to_string(report.totalInvalid) + " invalid, " +
           std::to_string(report.duplicateRows) + " duplicate rows";
}

std::string ColumnStatisticsJob::name() const {
    return "Statistics for column #" + std::to_string(m_col + 1);
}

std::string ColumnStatisticsJob::run(const Dataset& data) {
    if (m_col >= data.columnCount())
        return "column no longer exists";
    ColumnSummary s = Statistics::summarize(data.numericValues(m_col));
    return data.columnName(m_col) + ": mean=" + Dataset::formatNumber(s.mean, 3) +
           ", median=" + Dataset::formatNumber(s.median, 3) +
           ", std=" + Dataset::formatNumber(s.stdDev, 3) +
           ", min=" + Dataset::formatNumber(s.min, 3) +
           ", max=" + Dataset::formatNumber(s.max, 3);
}

std::string CorrelationJob::run(const Dataset& data) {
    CorrelationMatrix matrix = Correlation::computeMatrix(data);
    Graph graph;
    Correlation::buildGraph(matrix, m_threshold, graph);
    DynamicArray<int> clusters;
    int clusterCount = graph.connectedComponents(clusters);
    std::string text = std::to_string(matrix.size()) + " numeric columns, " +
                       std::to_string(graph.edgeCount()) + " strong links, " +
                       std::to_string(clusterCount) + " clusters";
    DynamicArray<StrongPair> pairs = Correlation::strongPairs(matrix, m_threshold);
    for (int i = 0; i < pairs.size() && i < 3; ++i)
        text += "\n      " + pairs[i].first + " <-> " + pairs[i].second +
                "  r=" + Dataset::formatNumber(pairs[i].r, 3);
    return text;
}

std::string TopKJob::name() const {
    return "Top-" + std::to_string(m_k) + " ranking for column #" + std::to_string(m_col + 1);
}

std::string TopKJob::run(const Dataset& data) {
    if (m_col >= data.columnCount())
        return "column no longer exists";
    DynamicArray<RankedItem> top = TopK::largest(data, m_col, m_k);
    std::string text = data.columnName(m_col) + ":";
    for (int i = 0; i < top.size(); ++i)
        text += " " + Dataset::formatNumber(top[i].value, 2);
    return text;
}

// ---------------------------- processor -----------------------------------

BatchProcessor::~BatchProcessor() {
    clear();
}

void BatchProcessor::addJob(BatchJob* job) {
    m_queue.enqueue(job);
    ++m_totalJobs;
}

void BatchProcessor::addFullAnalysis(const Dataset& data) {
    addJob(new ValidationJob());
    DynamicArray<int> numeric = data.numericColumnIndices();
    for (int i = 0; i < numeric.size(); ++i)
        addJob(new ColumnStatisticsJob(numeric[i]));
    addJob(new CorrelationJob(Correlation::DEFAULT_THRESHOLD));
    if (!numeric.isEmpty())
        addJob(new TopKJob(numeric[numeric.size() - 1], 5));
}

std::string BatchProcessor::nextJobName() const {
    return hasPending() ? m_queue.front()->name() : std::string();
}

std::string BatchProcessor::runNext(const Dataset& data, std::string& jobName) {
    if (!hasPending())
        return "";
    BatchJob* job = m_queue.dequeue();
    jobName = job->name();
    std::string output = job->run(data);   // virtual call
    delete job;
    ++m_finishedJobs;
    if (!hasPending()) {                   // batch done -> reset the counters
        m_totalJobs = 0;
        m_finishedJobs = 0;
    }
    return output;
}

void BatchProcessor::clear() {
    while (!m_queue.isEmpty())
        delete m_queue.dequeue();
    m_totalJobs = 0;
    m_finishedJobs = 0;
}
