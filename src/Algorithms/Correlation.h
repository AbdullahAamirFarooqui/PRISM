#ifndef PRISM_CORRELATION_H
#define PRISM_CORRELATION_H

#include "DSA/DynamicArray.h"
#include "DSA/Graph.h"
#include <string>

class Dataset;

// ---------------------------------------------------------------------------
// Correlation - Pearson correlation between numeric columns.
//
//   r = sum((x - mx)(y - my)) / sqrt( sum((x - mx)^2) * sum((y - my)^2) )
//
//   r = +1 perfect positive, 0 no linear relation, -1 perfect negative.
//
// The matrix can be turned into a Graph: every numeric column is a node and
// an edge is added when |r| >= threshold (default 0.70).
// ---------------------------------------------------------------------------

struct CorrelationMatrix {
    DynamicArray<std::string> names;          // column names
    DynamicArray<int> columnIndices;          // their index in the dataset
    DynamicArray<DynamicArray<double>> r;     // r[i][j]

    int size() const { return names.size(); }
};

struct StrongPair {
    std::string first;
    std::string second;
    double r = 0;
};

class Correlation {
public:
    // x and y must have the same length.
    static double pearson(const DynamicArray<double>& x, const DynamicArray<double>& y);

    // Pearson between two dataset columns, using only rows where BOTH are valid.
    static double pearsonColumns(const Dataset& data, int colX, int colY, int* pairsUsed = nullptr);

    static CorrelationMatrix computeMatrix(const Dataset& data);

    static void buildGraph(const CorrelationMatrix& matrix, double threshold, Graph& graph);

    // Pairs with |r| >= threshold, strongest first.
    static DynamicArray<StrongPair> strongPairs(const CorrelationMatrix& matrix, double threshold);

    // Simple linear regression y = slope * x + intercept (used for trend lines).
    static bool linearFit(const DynamicArray<double>& x, const DynamicArray<double>& y,
                          double& slope, double& intercept);

    static std::string describe(double r);    // "strong positive", "weak negative"...

    static constexpr double DEFAULT_THRESHOLD = 0.70;
};

#endif // PRISM_CORRELATION_H
