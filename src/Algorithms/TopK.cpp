#include "Algorithms/TopK.h"
#include "Data/Dataset.h"
#include "DSA/Heap.h"

// Shared worker: keeps the K "best" items in a heap.
//   wantLargest = true  -> min-heap, root = weakest of the K largest
//   wantLargest = false -> max-heap, root = weakest of the K smallest
static DynamicArray<RankedItem> selectTopK(const DynamicArray<RankedItem>& items, int k, bool wantLargest) {
    DynamicArray<RankedItem> result;
    if (k <= 0)
        return result;

    Heap<RankedItem> heap(wantLargest);   // min-heap when we want the largest
    for (int i = 0; i < items.size(); ++i) {
        if (heap.size() < k) {
            heap.push(items[i]);
        } else {
            bool better = wantLargest ? (heap.top() < items[i]) : (items[i] < heap.top());
            if (better) {
                heap.pop();
                heap.push(items[i]);
            }
        }
    }

    // Popping gives the weakest first, so fill the result from the back.
    result.resize(heap.size());
    for (int i = result.size() - 1; i >= 0; --i)
        result[i] = heap.pop();
    return result;
}

static DynamicArray<RankedItem> columnItems(const Dataset& data, int col) {
    DynamicArray<RankedItem> items;
    double value;
    for (int r = 0; r < data.rowCount(); ++r) {
        if (data.numberAt(r, col, value)) {
            RankedItem item;
            item.value = value;
            item.row = r;
            items.pushBack(item);
        }
    }
    return items;
}

DynamicArray<RankedItem> TopK::largest(const Dataset& data, int col, int k) {
    return selectTopK(columnItems(data, col), k, true);
}

DynamicArray<RankedItem> TopK::smallest(const Dataset& data, int col, int k) {
    return selectTopK(columnItems(data, col), k, false);
}

DynamicArray<RankedItem> TopK::largest(const DynamicArray<double>& values, int k) {
    DynamicArray<RankedItem> items;
    for (int i = 0; i < values.size(); ++i) {
        RankedItem item;
        item.value = values[i];
        item.row = i;
        items.pushBack(item);
    }
    return selectTopK(items, k, true);
}
