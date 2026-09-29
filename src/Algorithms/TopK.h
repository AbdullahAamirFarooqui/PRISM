#ifndef PRISM_TOPK_H
#define PRISM_TOPK_H

#include "DSA/DynamicArray.h"

class Dataset;

// ---------------------------------------------------------------------------
// TopK - heap based ranking.
//
// To get the K LARGEST values we keep a MIN-heap of size K:
//   - the root is the smallest of the current "top K"
//   - a new value bigger than the root replaces it
// Total time O(n log K) instead of O(n log n) for sorting everything.
// (For the K smallest values we do the opposite with a MAX-heap.)
// ---------------------------------------------------------------------------

struct RankedItem {
    double value = 0;
    int row = -1;
    bool operator<(const RankedItem& other) const { return value < other.value; }
};

class TopK {
public:
    static DynamicArray<RankedItem> largest(const Dataset& data, int col, int k);
    static DynamicArray<RankedItem> smallest(const Dataset& data, int col, int k);

    // Same idea on a plain array (row = index in the array).
    static DynamicArray<RankedItem> largest(const DynamicArray<double>& values, int k);
};

#endif // PRISM_TOPK_H
