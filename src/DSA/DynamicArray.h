#ifndef PRISM_DYNAMIC_ARRAY_H
#define PRISM_DYNAMIC_ARRAY_H

#include <stdexcept>

// ---------------------------------------------------------------------------
// DynamicArray<T>
// A growable array (our own replacement for std::vector).
//
//  - pushBack   : amortized O(1)  (capacity doubles when full)
//  - operator[] : O(1)
//  - insertAt / removeAt : O(n)   (elements have to be shifted)
//
// NOTE: DynamicArray is a *template*, so the whole implementation must live
// in the header file. The compiler needs to see the code to generate a version
// for every type we use (DynamicArray<int>, DynamicArray<double>, ...).
// ---------------------------------------------------------------------------
template <typename T>
class DynamicArray {
public:
    DynamicArray() : m_data(nullptr), m_size(0), m_capacity(0) {}

    explicit DynamicArray(int initialCapacity) : m_data(nullptr), m_size(0), m_capacity(0) {
        reserve(initialCapacity);
    }

    // ----- Rule of three: copy constructor, copy assignment, destructor -----
    DynamicArray(const DynamicArray& other) : m_data(nullptr), m_size(0), m_capacity(0) {
        copyFrom(other);
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] m_data;
            m_data = nullptr;
            m_size = m_capacity = 0;
            copyFrom(other);
        }
        return *this;
    }

    // Move operations just "steal" the buffer of a temporary array (fast returns).
    DynamicArray(DynamicArray&& other) noexcept
        : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
        other.m_data = nullptr;
        other.m_size = other.m_capacity = 0;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] m_data;
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_data = nullptr;
            other.m_size = other.m_capacity = 0;
        }
        return *this;
    }

    ~DynamicArray() { delete[] m_data; }

    // ----- Adding / removing -----
    void pushBack(const T& value) {
        if (m_size == m_capacity)
            grow();
        m_data[m_size++] = value;
    }

    void popBack() {
        if (m_size == 0)
            throw std::out_of_range("DynamicArray::popBack on empty array");
        --m_size;
    }

    void insertAt(int index, const T& value) {
        if (index < 0 || index > m_size)
            throw std::out_of_range("DynamicArray::insertAt index out of range");
        if (m_size == m_capacity)
            grow();
        for (int i = m_size; i > index; --i)   // shift right
            m_data[i] = m_data[i - 1];
        m_data[index] = value;
        ++m_size;
    }

    void removeAt(int index) {
        checkIndex(index);
        for (int i = index; i < m_size - 1; ++i)   // shift left
            m_data[i] = m_data[i + 1];
        --m_size;
    }

    void clear() { m_size = 0; }

    // Makes sure the array can hold at least newCapacity elements.
    void reserve(int newCapacity) {
        if (newCapacity <= m_capacity)
            return;
        T* newData = new T[newCapacity];
        for (int i = 0; i < m_size; ++i)
            newData[i] = m_data[i];
        delete[] m_data;
        m_data = newData;
        m_capacity = newCapacity;
    }

    // Changes the size; new slots are filled with fillValue.
    void resize(int newSize, const T& fillValue = T()) {
        if (newSize < 0)
            newSize = 0;
        reserve(newSize);
        for (int i = m_size; i < newSize; ++i)
            m_data[i] = fillValue;
        m_size = newSize;
    }

    // ----- Access -----
    T& operator[](int index) { return m_data[index]; }              // no bounds check (fast)
    const T& operator[](int index) const { return m_data[index]; }

    T& at(int index) { checkIndex(index); return m_data[index]; }  // bounds checked
    const T& at(int index) const { checkIndex(index); return m_data[index]; }

    T& back() { checkIndex(m_size - 1); return m_data[m_size - 1]; }
    const T& back() const { checkIndex(m_size - 1); return m_data[m_size - 1]; }

    void swap(int i, int j) {
        T temp = m_data[i];
        m_data[i] = m_data[j];
        m_data[j] = temp;
    }

    // Linear search, returns -1 if not found. (Requires T to support ==)
    int indexOf(const T& value) const {
        for (int i = 0; i < m_size; ++i)
            if (m_data[i] == value)
                return i;
        return -1;
    }

    bool contains(const T& value) const { return indexOf(value) != -1; }

    int size() const { return m_size; }
    int capacity() const { return m_capacity; }
    bool isEmpty() const { return m_size == 0; }

private:
    T* m_data;
    int m_size;       // number of elements stored
    int m_capacity;   // number of slots allocated

    void grow() { reserve(m_capacity == 0 ? 8 : m_capacity * 2); }

    void checkIndex(int index) const {
        if (index < 0 || index >= m_size)
            throw std::out_of_range("DynamicArray index out of range");
    }

    void copyFrom(const DynamicArray& other) {
        reserve(other.m_size);
        for (int i = 0; i < other.m_size; ++i)
            m_data[i] = other.m_data[i];
        m_size = other.m_size;
    }
};

#endif // PRISM_DYNAMIC_ARRAY_H
