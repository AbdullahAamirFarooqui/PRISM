#ifndef PRISM_BATCH_PROCESSOR_H
#define PRISM_BATCH_PROCESSOR_H

#include "Data/Dataset.h"
#include "DSA/Queue.h"
#include <string>

// ---------------------------------------------------------------------------
// Batch processing pipeline.
//
// BatchJob is an ABSTRACT class (interface). Each kind of job overrides
// name() and run(). The BatchProcessor stores BatchJob* in a circular Queue
// and runs them one by one in FIFO order - the GUI calls runNext() on a timer
// so it can update a progress bar between jobs.
// (Polymorphism: the processor never needs to know the concrete job type.)
// ---------------------------------------------------------------------------

class BatchJob {
public:
    virtual ~BatchJob() = default;
    virtual std::string name() const = 0;
    virtual std::string run(const Dataset& data) = 0;   // returns a text result
};

class ValidationJob : public BatchJob {
public:
    std::string name() const override { return "Validate dataset"; }
    std::string run(const Dataset& data) override;
};

class ColumnStatisticsJob : public BatchJob {
public:
    explicit ColumnStatisticsJob(int col) : m_col(col) {}
    std::string name() const override;
    std::string run(const Dataset& data) override;
private:
    int m_col;
};

class CorrelationJob : public BatchJob {
public:
    explicit CorrelationJob(double threshold) : m_threshold(threshold) {}
    std::string name() const override { return "Correlation matrix + relationship graph"; }
    std::string run(const Dataset& data) override;
private:
    double m_threshold;
};

class TopKJob : public BatchJob {
public:
    TopKJob(int col, int k) : m_col(col), m_k(k) {}
    std::string name() const override;
    std::string run(const Dataset& data) override;
private:
    int m_col;
    int m_k;
};

class BatchProcessor {
public:
    BatchProcessor() = default;
    ~BatchProcessor();

    BatchProcessor(const BatchProcessor&) = delete;
    BatchProcessor& operator=(const BatchProcessor&) = delete;

    void addJob(BatchJob* job);          // the processor takes ownership

    // Adds the standard "full analysis" pipeline for a dataset.
    void addFullAnalysis(const Dataset& data);

    bool hasPending() const { return !m_queue.isEmpty(); }
    int pendingCount() const { return m_queue.size(); }
    int totalJobs() const { return m_totalJobs; }
    int finishedJobs() const { return m_finishedJobs; }
    std::string nextJobName() const;

    // Runs the job at the front of the queue. Returns its text output.
    std::string runNext(const Dataset& data, std::string& jobName);

    void clear();

private:
    Queue<BatchJob*> m_queue;
    int m_totalJobs = 0;
    int m_finishedJobs = 0;
};

#endif // PRISM_BATCH_PROCESSOR_H
