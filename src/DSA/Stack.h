#ifndef PRISM_STACK_H
#define PRISM_STACK_H

#include "DSA/DynamicArray.h"
#include <stdexcept>

// ---------------------------------------------------------------------------
// Stack<T>  -  LIFO (last in, first out), built on top of DynamicArray.
//
// Used for the undo / redo engine.
//  - push / pop / top : O(1)
// ---------------------------------------------------------------------------
template <typename T>
class Stack {
public:
    void push(const T& value) { m_items.pushBack(value); }

    T pop() {
        if (isEmpty())
            throw std::out_of_range("Stack::pop on empty stack");
        T value = m_items.back();
        m_items.popBack();
        return value;
    }

    T& top() {
        if (isEmpty())
            throw std::out_of_range("Stack::top on empty stack");
        return m_items.back();
    }

    const T& top() const {
        if (isEmpty())
            throw std::out_of_range("Stack::top on empty stack");
        return m_items.back();
    }

    // Removes the OLDEST element (bottom of the stack). O(n).
    // Only used to cap the size of the undo history so memory does not explode.
    void removeBottom() {
        if (!isEmpty())
            m_items.removeAt(0);
    }

    int size() const { return m_items.size(); }
    bool isEmpty() const { return m_items.isEmpty(); }
    void clear() { m_items.clear(); }

private:
    DynamicArray<T> m_items;   // the end of the array is the top of the stack
};

#endif // PRISM_STACK_H
