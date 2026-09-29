#include "Algorithms/Sorting.h"
#include <chrono>

// ------------------------------- Sorter -----------------------------------

SortStats Sorter::sort(DynamicArray<double>& data) {
    m_comparisons = 0;
    m_swaps = 0;

    auto start = std::chrono::high_resolution_clock::now();
    sortImpl(data);                                    // <- virtual call
    auto end = std::chrono::high_resolution_clock::now();

    SortStats stats;
    stats.algorithm = name();
    stats.complexity = complexity();
    stats.comparisons = m_comparisons;
    stats.swaps = m_swaps;
    stats.elements = data.size();
    stats.milliseconds = std::chrono::duration<double, std::milli>(end - start).count();
    return stats;
}

// ----------------------------- Bubble Sort --------------------------------
// Repeatedly swaps neighbours that are in the wrong order. Stops early if a
// full pass made no swap (array already sorted).
void BubbleSorter::sortImpl(DynamicArray<double>& data) {
    int n = data.size();
    for (int pass = 0; pass < n - 1; ++pass) {
        bool swapped = false;
        for (int i = 0; i < n - 1 - pass; ++i) {
            if (less(data[i + 1], data[i])) {
                swapItems(data, i, i + 1);
                swapped = true;
            }
        }
        if (!swapped)
            break;
    }
}

// ---------------------------- Selection Sort ------------------------------
// Finds the smallest remaining element and puts it at the front.
void SelectionSorter::sortImpl(DynamicArray<double>& data) {
    int n = data.size();
    for (int i = 0; i < n - 1; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < n; ++j)
            if (less(data[j], data[minIndex]))
                minIndex = j;
        if (minIndex != i)
            swapItems(data, i, minIndex);
    }
}

// ---------------------------- Insertion Sort ------------------------------
// Takes each element and shifts it left until it is in the right place.
// (Each shift is counted as a "swap"/move.)
void InsertionSorter::sortImpl(DynamicArray<double>& data) {
    int n = data.size();
    for (int i = 1; i < n; ++i) {
        double key = data[i];
        int j = i - 1;
        while (j >= 0 && less(key, data[j])) {
            data[j + 1] = data[j];
            ++m_swaps;
            --j;
        }
        data[j + 1] = key;
    }
}

// ------------------------------ Merge Sort --------------------------------
// Divide the array in halves, sort each half recursively, merge them.
void MergeSorter::sortImpl(DynamicArray<double>& data) {
    if (data.size() < 2)
        return;
    DynamicArray<double> temp;
    temp.resize(data.size());
    mergeSortRec(data, temp, 0, data.size() - 1);
}

void MergeSorter::mergeSortRec(DynamicArray<double>& data, DynamicArray<double>& temp, int left, int right) {
    if (left >= right)
        return;
    int mid = left + (right - left) / 2;
    mergeSortRec(data, temp, left, mid);
    mergeSortRec(data, temp, mid + 1, right);
    merge(data, temp, left, mid, right);
}

void MergeSorter::merge(DynamicArray<double>& data, DynamicArray<double>& temp, int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (less(data[j], data[i])) temp[k++] = data[j++];
        else temp[k++] = data[i++];            // "<=" keeps the sort stable
    }
    while (i <= mid) temp[k++] = data[i++];
    while (j <= right) temp[k++] = data[j++];
    for (int x = left; x <= right; ++x) {
        data[x] = temp[x];
        ++m_swaps;                               // counted as a move
    }
}

// ------------------------------ Quick Sort --------------------------------
// Picks a pivot (median of three, avoids the O(n^2) case on sorted input),
// puts smaller elements left and bigger ones right, then recurses.
void QuickSorter::sortImpl(DynamicArray<double>& data) {
    if (data.size() > 1)
        quickSortRec(data, 0, data.size() - 1);
}

void QuickSorter::quickSortRec(DynamicArray<double>& data, int low, int high) {
    while (low < high) {
        int p = partition(data, low, high);
        // Recurse into the smaller part, loop on the bigger one:
        // keeps recursion depth O(log n).
        if (p - low < high - p) {
            quickSortRec(data, low, p - 1);
            low = p + 1;
        } else {
            quickSortRec(data, p + 1, high);
            high = p - 1;
        }
    }
}

int QuickSorter::partition(DynamicArray<double>& data, int low, int high) {
    int mid = low + (high - low) / 2;
    // Order data[low], data[mid], data[high] so the median ends up at mid.
    if (less(data[mid], data[low])) swapItems(data, mid, low);
    if (less(data[high], data[low])) swapItems(data, high, low);
    if (less(data[high], data[mid])) swapItems(data, high, mid);
    swapItems(data, mid, high);                  // move pivot to the end
    double pivot = data[high];

    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (less(data[j], pivot)) {
            ++i;
            if (i != j)
                swapItems(data, i, j);
        }
    }
    swapItems(data, i + 1, high);                // pivot into its final place
    return i + 1;
}

// ------------------------------- Heap Sort --------------------------------
// Build a max-heap, then repeatedly move the largest element to the end.
void HeapSorter::sortImpl(DynamicArray<double>& data) {
    int n = data.size();
    for (int i = n / 2 - 1; i >= 0; --i)
        heapify(data, n, i);
    for (int end = n - 1; end > 0; --end) {
        swapItems(data, 0, end);
        heapify(data, end, 0);
    }
}

void HeapSorter::heapify(DynamicArray<double>& data, int size, int root) {
    while (true) {
        int largest = root;
        int left = 2 * root + 1;
        int right = 2 * root + 2;
        if (left < size && less(data[largest], data[left])) largest = left;
        if (right < size && less(data[largest], data[right])) largest = right;
        if (largest == root)
            return;
        swapItems(data, root, largest);
        root = largest;
    }
}

// ------------------------------- Sorting ----------------------------------

DynamicArray<Sorter*> Sorting::createAll() {
    DynamicArray<Sorter*> sorters;
    sorters.pushBack(new BubbleSorter());
    sorters.pushBack(new SelectionSorter());
    sorters.pushBack(new InsertionSorter());
    sorters.pushBack(new MergeSorter());
    sorters.pushBack(new QuickSorter());
    sorters.pushBack(new HeapSorter());
    return sorters;
}

void Sorting::destroyAll(DynamicArray<Sorter*>& sorters) {
    for (int i = 0; i < sorters.size(); ++i)
        delete sorters[i];
    sorters.clear();
}

void Sorting::sortAscending(DynamicArray<double>& data) {
    MergeSorter sorter;
    sorter.sort(data);
}

bool Sorting::isSorted(const DynamicArray<double>& data) {
    for (int i = 1; i < data.size(); ++i)
        if (data[i] < data[i - 1])
            return false;
    return true;
}
