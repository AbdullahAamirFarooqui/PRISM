#include "Algorithms/Correlation.h"
#include "Data/Dataset.h"
#include <cmath>
#include <limits>

double Correlation::pearson(const DynamicArray<double>& x, const DynamicArray<double>& y) {
    int n = x.size() < y.size() ? x.size() : y.size();
    if (n < 2)
        return std::numeric_limits<double>::quiet_NaN();

    double meanX = 0, meanY = 0;
    for (int i = 0; i < n; ++i) {
        meanX += x[i];
        meanY += y[i];
    }
    meanX /= n;
    meanY /= n;

    double covariance = 0, varX = 0, varY = 0;
    for (int i = 0; i < n; ++i) {
        double dx = x[i] - meanX;
        double dy = y[i] - meanY;
        covariance += dx * dy;
        varX += dx * dx;
        varY += dy * dy;
    }
    if (varX == 0 || varY == 0)         // a constant column has no correlation
        return std::numeric_limits<double>::quiet_NaN();
    return covariance / std::sqrt(varX * varY);
}

double Correlation::pearsonColumns(const Dataset& data, int colX, int colY, int* pairsUsed) {
    DynamicArray<double> xs, ys;
    double x, y;
    for (int r = 0; r < data.rowCount(); ++r) {
        if (data.numberAt(r, colX, x) && data.numberAt(r, colY, y)) {
            xs.pushBack(x);
            ys.pushBack(y);
        }
    }
    if (pairsUsed != nullptr)
        *pairsUsed = xs.size();
    return pearson(xs, ys);
}

CorrelationMatrix Correlation::computeMatrix(const Dataset& data) {
    CorrelationMatrix matrix;
    matrix.columnIndices = data.numericColumnIndices();
    int m = matrix.columnIndices.size();
    for (int i = 0; i < m; ++i)
        matrix.names.pushBack(data.columnName(matrix.columnIndices[i]));

    // m x m table, the diagonal is always 1. The matrix is symmetric, so we
    // only compute the upper half and mirror it: O(m^2 * n).
    matrix.r.resize(m);
    for (int i = 0; i < m; ++i)
        matrix.r[i].resize(m, 0.0);

    for (int i = 0; i < m; ++i) {
        matrix.r[i][i] = 1.0;
        for (int j = i + 1; j < m; ++j) {
            double value = pearsonColumns(data, matrix.columnIndices[i], matrix.columnIndices[j]);
            matrix.r[i][j] = value;
            matrix.r[j][i] = value;
        }
    }
    return matrix;
}

void Correlation::buildGraph(const CorrelationMatrix& matrix, double threshold, Graph& graph) {
    graph.clear();
    for (int i = 0; i < matrix.size(); ++i)
        graph.addNode(matrix.names[i]);
    for (int i = 0; i < matrix.size(); ++i)
        for (int j = i + 1; j < matrix.size(); ++j) {
            double r = matrix.r[i][j];
            if (!std::isnan(r) && std::fabs(r) >= threshold)
                graph.addEdge(i, j, r);
        }
}

DynamicArray<StrongPair> Correlation::strongPairs(const CorrelationMatrix& matrix, double threshold) {
    DynamicArray<StrongPair> pairs;
    for (int i = 0; i < matrix.size(); ++i)
        for (int j = i + 1; j < matrix.size(); ++j) {
            double r = matrix.r[i][j];
            if (!std::isnan(r) && std::fabs(r) >= threshold) {
                StrongPair p;
                p.first = matrix.names[i];
                p.second = matrix.names[j];
                p.r = r;
                pairs.pushBack(p);
            }
        }
    // insertion sort by |r| descending (lists are tiny)
    for (int i = 1; i < pairs.size(); ++i) {
        StrongPair key = pairs[i];
        int j = i - 1;
        while (j >= 0 && std::fabs(pairs[j].r) < std::fabs(key.r)) {
            pairs[j + 1] = pairs[j];
            --j;
        }
        pairs[j + 1] = key;
    }
    return pairs;
}

bool Correlation::linearFit(const DynamicArray<double>& x, const DynamicArray<double>& y,
                            double& slope, double& intercept) {
    int n = x.size() < y.size() ? x.size() : y.size();
    if (n < 2)
        return false;
    double meanX = 0, meanY = 0;
    for (int i = 0; i < n; ++i) { meanX += x[i]; meanY += y[i]; }
    meanX /= n;
    meanY /= n;
    double sxy = 0, sxx = 0;
    for (int i = 0; i < n; ++i) {
        sxy += (x[i] - meanX) * (y[i] - meanY);
        sxx += (x[i] - meanX) * (x[i] - meanX);
    }
    if (sxx == 0)
        return false;
    slope = sxy / sxx;
    intercept = meanY - slope * meanX;
    return true;
}

std::string Correlation::describe(double r) {
    if (std::isnan(r))
        return "undefined";
    double a = std::fabs(r);
    std::string strength = a >= 0.7 ? "strong" : a >= 0.4 ? "moderate" : a >= 0.2 ? "weak" : "no";
    if (strength == "no")
        return "no clear relation";
    return strength + (r > 0 ? " positive" : " negative");
}
