#ifndef PRISM_QUEUE_H
#define PRISM_QUEUE_H

#include <stdexcept>

// ---------------------------------------------------------------------------
// Queue<T>  -  FIFO (first in, first out) using a CIRCULAR array.
//
// Used for: the batch processing pipeline and BFS in the Graph.
//
// The array wraps around: when the back reaches the end of the buffer it
// continues from index 0, so no shifting is ever needed.
//  - enqueue / dequeue / front : O(1)   (enqueue is amortized when growing)
// ---------------------------------------------------------------------------
template <typename T>
class Queue {
public:
    explicit Queue(int initialCapacity = 8)
        : m_data(new T[initialCapacity > 0 ? initialCapacity : 8]),
          m_capacity(initialCapacity > 0 ? initialCapacity : 8),
          m_front(0), m_count(0) {}

    ~Queue() { delete[] m_data; }

    // A queue owns a raw buffer; copying is not needed in PRISM, so we forbid it.
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void enqueue(const T& value) {
        if (m_count == m_capacity)
            grow();
        int back = (m_front + m_count) % m_capacity;   // wrap around
        m_data[back] = value;
        ++m_count;
    }

    T dequeue() {
        if (isEmpty())
            throw std::out_of_range("Queue::dequeue on empty queue");
        T value = m_data[m_front];
        m_front = (m_front + 1) % m_capacity;          // wrap around
        --m_count;
        return value;
    }

    const T& front() const {
        if (isEmpty())
            throw std::out_of_range("Queue::front on empty queue");
        return m_data[m_front];
    }

    int size() const { return m_count; }
    int capacity() const { return m_capacity; }
    bool isEmpty() const { return m_count == 0; }
    void clear() { m_front = 0; m_count = 0; }

private:
    T* m_data;
    int m_capacity;
    int m_front;   // index of the first element
    int m_count;   // number of stored elements

    // Doubles the buffer and "unrolls" the circle so the front is at index 0.
    void grow() {
        int newCapacity = m_capacity * 2;
        T* newData = new T[newCapacity];
        for (int i = 0; i < m_count; ++i)
            newData[i] = m_data[(m_front + i) % m_capacity];
        delete[] m_data;
        m_data = newData;
        m_capacity = newCapacity;
        m_front = 0;
    }
};

#endif // PRISM_QUEUE_H
