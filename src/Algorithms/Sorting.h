#ifndef PRISM_SORTING_H
#define PRISM_SORTING_H

#include "DSA/DynamicArray.h"
#include <string>

// ---------------------------------------------------------------------------
// Instrumented sorting algorithms.
//
// OOP design (Template Method pattern):
//   Sorter            - abstract base class. sort() starts a timer, resets the
//                       counters and calls the virtual sortImpl().
//   BubbleSorter ...  - each algorithm overrides sortImpl() and name().
//
// So the Algorithm Lab can keep a list of Sorter* and treat them all the same.
//
// Counters:
//   comparisons = number of times two elements were compared
//   swaps       = number of swaps / element moves (writes) performed
// ---------------------------------------------------------------------------

struct SortStats {
    std::string algorithm;
    std::string complexity;
    long long comparisons = 0;
    long long swaps = 0;
    double milliseconds = 0.0;
    int elements = 0;
};

class Sorter {
public:
    virtual ~Sorter() = default;

    // Sorts 'data' in ascending order and returns the measured statistics.
    SortStats sort(DynamicArray<double>& data);

    virtual std::string name() const = 0;
    virtual std::string complexity() const = 0;

protected:
    virtual void sortImpl(DynamicArray<double>& data) = 0;

    // Helpers that also update the counters
    bool less(double a, double b) { ++m_comparisons; return a < b; }
    void swapItems(DynamicArray<double>& data, int i, int j) { ++m_swaps; data.swap(i, j); }

    long long m_comparisons = 0;
    long long m_swaps = 0;
};

class BubbleSorter : public Sorter {
public:
    std::string name() const override { return "Bubble Sort"; }
    std::string complexity() const override { return "O(n^2)"; }
protected:
    void sortImpl(DynamicArray<double>& data) override;
};

class SelectionSorter : public Sorter {
public:
    std::string name() const override { return "Selection Sort"; }
    std::string complexity() const override { return "O(n^2)"; }
protected:
    void sortImpl(DynamicArray<double>& data) override;
};

class InsertionSorter : public Sorter {
public:
    std::string name() const override { return "Insertion Sort"; }
    std::string complexity() const override { return "O(n^2)"; }
protected:
    void sortImpl(DynamicArray<double>& data) override;
};

class MergeSorter : public Sorter {
public:
    std::string name() const override { return "Merge Sort"; }
    std::string complexity() const override { return "O(n log n)"; }
protected:
    void sortImpl(DynamicArray<double>& data) override;
private:
    void mergeSortRec(DynamicArray<double>& data, DynamicArray<double>& temp, int left, int right);
    void merge(DynamicArray<double>& data, DynamicArray<double>& temp, int left, int mid, int right);
};

class QuickSorter : public Sorter {
public:
    std::string name() const override { return "Quick Sort"; }
    std::string complexity() const override { return "O(n log n) avg"; }
protected:
    void sortImpl(DynamicArray<double>& data) override;
private:
    void quickSortRec(DynamicArray<double>& data, int low, int high);
    int partition(DynamicArray<double>& data, int low, int high);
};

class HeapSorter : public Sorter {
public:
    std::string name() const override { return "Heap Sort"; }
    std::string complexity() const override { return "O(n log n)"; }
protected:
    void sortImpl(DynamicArray<double>& data) override;
private:
    void heapify(DynamicArray<double>& data, int size, int root);
};

// Small helper namespace-like class.
class Sorting {
public:
    // Creates one object of every algorithm. The caller must delete them
    // (or call Sorting::destroyAll).
    static DynamicArray<Sorter*> createAll();
    static void destroyAll(DynamicArray<Sorter*>& sorters);

    // Plain fast sort (merge sort) used by other modules, e.g. Statistics.
    static void sortAscending(DynamicArray<double>& data);
    static bool isSorted(const DynamicArray<double>& data);
};

#endif // PRISM_SORTING_H
