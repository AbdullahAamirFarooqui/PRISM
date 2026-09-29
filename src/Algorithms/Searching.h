#ifndef PRISM_SEARCHING_H
#define PRISM_SEARCHING_H

#include "DSA/DynamicArray.h"
#include "DSA/HashTable.h"
#include <string>

// ---------------------------------------------------------------------------
// Searching algorithms with comparison counters (for the Algorithm Lab).
//
//  Linear search : O(n)      - works on any array
//  Binary search : O(log n)  - array MUST be sorted
//  Hash search   : O(1) avg  - uses the custom HashTable
// ---------------------------------------------------------------------------

struct SearchResult {
    std::string algorithm;
    bool found = false;
    int index = -1;              // position in the array (or first matching row)
    int matches = 0;             // how many rows have that value (hash search)
    long long comparisons = 0;
    double microseconds = 0.0;
};

class Searching {
public:
    static SearchResult linearSearch(const DynamicArray<double>& data, double target);
    static SearchResult binarySearch(const DynamicArray<double>& sortedData, double target);
    static SearchResult hashSearch(const HashTable& table, const std::string& key);
};

#endif // PRISM_SEARCHING_H
