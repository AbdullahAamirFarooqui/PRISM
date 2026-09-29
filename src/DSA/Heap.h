#ifndef PRISM_HEAP_H
#define PRISM_HEAP_H

#include "DSA/DynamicArray.h"
#include <stdexcept>

// ---------------------------------------------------------------------------
// Heap<T>  -  array-backed binary heap / priority queue.
//
// Can work as a MIN-heap (smallest on top) or a MAX-heap (largest on top).
// T must support operator<.
//
// For the element at index i:
//     parent = (i - 1) / 2      left child = 2i + 1      right child = 2i + 2
//
//  - push / pop : O(log n)
//  - top        : O(1)
//  - buildFrom  : O(n)   (bottom-up heapify)
//
// Used for Top-K ranking (e.g. the 10 strongest earthquakes).
// ---------------------------------------------------------------------------
template <typename T>
class Heap {
public:
    explicit Heap(bool isMinHeap = true) : m_isMinHeap(isMinHeap) {}

    void push(const T& value) {
        m_items.pushBack(value);
        siftUp(m_items.size() - 1);
    }

    T pop() {
        if (isEmpty())
            throw std::out_of_range("Heap::pop on empty heap");
        T topValue = m_items[0];
        m_items[0] = m_items.back();   // move last element to the root
        m_items.popBack();
        if (!isEmpty())
            siftDown(0);
        return topValue;
    }

    const T& top() const {
        if (isEmpty())
            throw std::out_of_range("Heap::top on empty heap");
        return m_items[0];
    }

    // Turns an unordered array into a heap in O(n).
    void buildFrom(const DynamicArray<T>& values) {
        m_items = values;
        for (int i = m_items.size() / 2 - 1; i >= 0; --i)
            siftDown(i);
    }

    int size() const { return m_items.size(); }
    bool isEmpty() const { return m_items.isEmpty(); }
    bool isMinHeap() const { return m_isMinHeap; }
    void clear() { m_items.clear(); }

private:
    DynamicArray<T> m_items;
    bool m_isMinHeap;

    // true if a should be closer to the top than b
    bool higherPriority(const T& a, const T& b) const {
        return m_isMinHeap ? (a < b) : (b < a);
    }

    void siftUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (!higherPriority(m_items[index], m_items[parent]))
                break;
            m_items.swap(index, parent);
            index = parent;
        }
    }

    void siftDown(int index) {
        int n = m_items.size();
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int best = index;
            if (left < n && higherPriority(m_items[left], m_items[best]))
                best = left;
            if (right < n && higherPriority(m_items[right], m_items[best]))
                best = right;
            if (best == index)
                break;
            m_items.swap(index, best);
            index = best;
        }
    }
};

#endif // PRISM_HEAP_H
