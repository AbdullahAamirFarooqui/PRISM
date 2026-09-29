#include "Algorithms/Statistics.h"
#include "Algorithms/Sorting.h"
#include "Data/Dataset.h"
#include "DSA/HashTable.h"
#include "DSA/Heap.h"
#include <cmath>
#include <limits>

static const double NOT_A_NUMBER = std::numeric_limits<double>::quiet_NaN();

double Statistics::sum(const DynamicArray<double>& values) {
    double total = 0.0;
    for (int i = 0; i < values.size(); ++i)
        total += values[i];
    return total;
}

double Statistics::mean(const DynamicArray<double>& values) {
    if (values.isEmpty())
        return NOT_A_NUMBER;
    return sum(values) / values.size();
}

double Statistics::median(const DynamicArray<double>& values) {
    return percentile(values, 50.0);
}

// Most frequent value. Works on a sorted copy: equal values are neighbours,
// so we only need to count "runs" of the same number.
double Statistics::mode(const DynamicArray<double>& values, int& frequency) {
    frequency = 0;
    if (values.isEmpty())
        return NOT_A_NUMBER;
    DynamicArray<double> sorted = values;
    Sorting::sortAscending(sorted);

    double bestValue = sorted[0];
    int bestCount = 1;
    int runCount = 1;
    for (int i = 1; i < sorted.size(); ++i) {
        runCount = (sorted[i] == sorted[i - 1]) ? runCount + 1 : 1;
        if (runCount > bestCount) {
            bestCount = runCount;
            bestValue = sorted[i];
        }
    }
    frequency = bestCount;
    return bestValue;
}

double Statistics::minimum(const DynamicArray<double>& values) {
    if (values.isEmpty())
        return NOT_A_NUMBER;
    double m = values[0];
    for (int i = 1; i < values.size(); ++i)
        if (values[i] < m) m = values[i];
    return m;
}

double Statistics::maximum(const DynamicArray<double>& values) {
    if (values.isEmpty())
        return NOT_A_NUMBER;
    double m = values[0];
    for (int i = 1; i < values.size(); ++i)
        if (values[i] > m) m = values[i];
    return m;
}

double Statistics::variance(const DynamicArray<double>& values) {
    if (values.isEmpty())
        return NOT_A_NUMBER;
    double mu = mean(values);
    double total = 0.0;
    for (int i = 0; i < values.size(); ++i)
        total += (values[i] - mu) * (values[i] - mu);
    return total / values.size();
}

double Statistics::standardDeviation(const DynamicArray<double>& values) {
    return std::sqrt(variance(values));
}

// Linear interpolation between the two closest ranks.
double Statistics::percentileSorted(const DynamicArray<double>& sortedValues, double p) {
    int n = sortedValues.size();
    if (n == 0)
        return NOT_A_NUMBER;
    if (p <= 0) return sortedValues[0];
    if (p >= 100) return sortedValues[n - 1];

    double position = (p / 100.0) * (n - 1);
    int lower = static_cast<int>(std::floor(position));
    int upper = static_cast<int>(std::ceil(position));
    double fraction = position - lower;
    return sortedValues[lower] + fraction * (sortedValues[upper] - sortedValues[lower]);
}

double Statistics::percentile(const DynamicArray<double>& values, double p) {
    DynamicArray<double> sorted = values;
    Sorting::sortAscending(sorted);
    return percentileSorted(sorted, p);
}

ColumnSummary Statistics::summarize(const DynamicArray<double>& values) {
    ColumnSummary s;
    s.count = values.size();
    if (s.count == 0) {
        s.mean = s.median = s.mode = s.min = s.max = s.range = NOT_A_NUMBER;
        s.variance = s.stdDev = s.q1 = s.q3 = s.iqr = s.sum = NOT_A_NUMBER;
        return s;
    }

    DynamicArray<double> sorted = values;
    Sorting::sortAscending(sorted);

    s.sum = sum(values);
    s.mean = s.sum / s.count;
    s.min = sorted[0];
    s.max = sorted[s.count - 1];
    s.range = s.max - s.min;
    s.median = percentileSorted(sorted, 50);
    s.q1 = percentileSorted(sorted, 25);
    s.q3 = percentileSorted(sorted, 75);
    s.iqr = s.q3 - s.q1;
    s.variance = variance(values);
    s.stdDev = std::sqrt(s.variance);
    s.mode = mode(values, s.modeFrequency);
    return s;
}

DynamicArray<FrequencyBin> Statistics::frequencyDistribution(const DynamicArray<double>& values, int binCount) {
    DynamicArray<FrequencyBin> bins;
    if (values.isEmpty() || binCount <= 0)
        return bins;

    double low = minimum(values);
    double high = maximum(values);
    double width = (high - low) / binCount;
    if (width == 0)             // all values identical -> one bin
        width = 1;

    for (int b = 0; b < binCount; ++b) {
        FrequencyBin bin;
        bin.from = low + b * width;
        bin.to = bin.from + width;
        bins.pushBack(bin);
    }
    for (int i = 0; i < values.size(); ++i) {
        int b = static_cast<int>((values[i] - low) / width);
        if (b >= binCount) b = binCount - 1;   // the maximum goes in the last bin
        if (b < 0) b = 0;
        bins[b].count++;
    }
    return bins;
}

// Helper so ValueCount can be used in a Heap (needs operator<).
struct CountEntry {
    int count = 0;
    std::string value;
    bool operator<(const CountEntry& other) const { return count < other.count; }
};

DynamicArray<ValueCount> Statistics::valueCounts(const Dataset& data, int col, int maxResults) {
    HashTable table(data.rowCount() + 11);
    for (int r = 0; r < data.rowCount(); ++r)
        if (!data.isMissing(r, col))
            table.insert(data.cell(r, col), r);

    DynamicArray<std::string> keys;
    table.keys(keys);

    // Max-heap on the count -> pop gives the most frequent values first.
    Heap<CountEntry> heap(false);
    DynamicArray<CountEntry> entries;
    for (int i = 0; i < keys.size(); ++i) {
        CountEntry e;
        e.value = keys[i];
        e.count = table.find(keys[i])->size();
        entries.pushBack(e);
    }
    heap.buildFrom(entries);

    DynamicArray<ValueCount> result;
    while (!heap.isEmpty() && result.size() < maxResults) {
        CountEntry e = heap.pop();
        ValueCount vc;
        vc.value = e.value;
        vc.count = e.count;
        result.pushBack(vc);
    }
    return result;
}
