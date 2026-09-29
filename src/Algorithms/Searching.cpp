#include "Algorithms/Searching.h"
#include <chrono>

using Clock = std::chrono::high_resolution_clock;

static double microsecondsSince(Clock::time_point start) {
    return std::chrono::duration<double, std::micro>(Clock::now() - start).count();
}

SearchResult Searching::linearSearch(const DynamicArray<double>& data, double target) {
    SearchResult result;
    result.algorithm = "Linear Search";
    auto start = Clock::now();

    for (int i = 0; i < data.size(); ++i) {
        ++result.comparisons;
        if (data[i] == target) {
            result.found = true;
            result.index = i;
            break;
        }
    }

    result.microseconds = microsecondsSince(start);
    result.matches = result.found ? 1 : 0;
    return result;
}

SearchResult Searching::binarySearch(const DynamicArray<double>& sortedData, double target) {
    SearchResult result;
    result.algorithm = "Binary Search";
    auto start = Clock::now();

    int low = 0;
    int high = sortedData.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        ++result.comparisons;
        if (sortedData[mid] == target) {
            result.found = true;
            result.index = mid;
            break;
        }
        if (sortedData[mid] < target) low = mid + 1;   // go right
        else high = mid - 1;                          // go left
    }

    result.microseconds = microsecondsSince(start);
    result.matches = result.found ? 1 : 0;
    return result;
}

SearchResult Searching::hashSearch(const HashTable& table, const std::string& key) {
    SearchResult result;
    result.algorithm = "Hash Search";
    auto start = Clock::now();

    int comparisons = 0;
    const DynamicArray<int>* rows = table.find(key, comparisons);

    result.microseconds = microsecondsSince(start);
    result.comparisons = comparisons;
    if (rows != nullptr && !rows->isEmpty()) {
        result.found = true;
        result.index = (*rows)[0];
        result.matches = rows->size();
    }
    return result;
}
