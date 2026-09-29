#ifndef PRISM_STATISTICS_H
#define PRISM_STATISTICS_H

#include "DSA/DynamicArray.h"
#include <string>

class Dataset;

// ---------------------------------------------------------------------------
// Statistics engine - descriptive statistics written from scratch.
//
//  mean     = (1/n) * sum(x)
//  variance = (1/n) * sum((x - mean)^2)          (population variance)
//  stdDev   = sqrt(variance)
//  median / quartiles / percentiles use a sorted copy (linear interpolation).
// ---------------------------------------------------------------------------

struct ColumnSummary {
    int count = 0;
    double mean = 0, median = 0, mode = 0;
    int modeFrequency = 0;
    double min = 0, max = 0, range = 0;
    double variance = 0, stdDev = 0;
    double q1 = 0, q3 = 0, iqr = 0;
    double sum = 0;
};

struct FrequencyBin {
    double from = 0;
    double to = 0;
    int count = 0;
};

struct ValueCount {          // for text columns: "Karachi" appears 120 times
    std::string value;
    int count = 0;
};

class Statistics {
public:
    static double sum(const DynamicArray<double>& values);
    static double mean(const DynamicArray<double>& values);
    static double median(const DynamicArray<double>& values);
    static double mode(const DynamicArray<double>& values, int& frequency);
    static double minimum(const DynamicArray<double>& values);
    static double maximum(const DynamicArray<double>& values);
    static double variance(const DynamicArray<double>& values);
    static double standardDeviation(const DynamicArray<double>& values);

    // p between 0 and 100. 'sortedValues' must already be sorted.
    static double percentileSorted(const DynamicArray<double>& sortedValues, double p);
    static double percentile(const DynamicArray<double>& values, double p);

    // Everything above in one call (sorts only once).
    static ColumnSummary summarize(const DynamicArray<double>& values);

    // Splits [min, max] into binCount equal ranges and counts values per range.
    static DynamicArray<FrequencyBin> frequencyDistribution(const DynamicArray<double>& values, int binCount);

    // Most frequent text values of a column (uses the HashTable), largest first.
    static DynamicArray<ValueCount> valueCounts(const Dataset& data, int col, int maxResults);
};

#endif // PRISM_STATISTICS_H
