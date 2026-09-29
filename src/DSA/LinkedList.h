#ifndef PRISM_LINKED_LIST_H
#define PRISM_LINKED_LIST_H

#include <stdexcept>

// ---------------------------------------------------------------------------
// LinkedList<T>  -  a doubly linked list.
//
// Used for:  the analysis history log  and  the graph adjacency lists.
//
//  - pushBack / pushFront / popBack / popFront : O(1)
//  - traversal                                 : O(n)
//
// How to loop over the list:
//     for (const LinkedList<int>::Node* n = list.first(); n != nullptr; n = n->next)
//         use(n->data);
//
// (Template -> implementation stays in the header.)
// ---------------------------------------------------------------------------
template <typename T>
class LinkedList {
public:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        explicit Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
    };

    LinkedList() : m_head(nullptr), m_tail(nullptr), m_size(0) {}

    LinkedList(const LinkedList& other) : m_head(nullptr), m_tail(nullptr), m_size(0) {
        copyFrom(other);
    }

    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    ~LinkedList() { clear(); }

    void pushBack(const T& value) {
        Node* node = new Node(value);
        if (m_tail == nullptr) {
            m_head = m_tail = node;
        } else {
            node->prev = m_tail;
            m_tail->next = node;
            m_tail = node;
        }
        ++m_size;
    }

    void pushFront(const T& value) {
        Node* node = new Node(value);
        if (m_head == nullptr) {
            m_head = m_tail = node;
        } else {
            node->next = m_head;
            m_head->prev = node;
            m_head = node;
        }
        ++m_size;
    }

    void popFront() {
        if (m_head == nullptr)
            throw std::out_of_range("LinkedList::popFront on empty list");
        Node* old = m_head;
        m_head = m_head->next;
        if (m_head != nullptr) m_head->prev = nullptr;
        else m_tail = nullptr;
        delete old;
        --m_size;
    }

    void popBack() {
        if (m_tail == nullptr)
            throw std::out_of_range("LinkedList::popBack on empty list");
        Node* old = m_tail;
        m_tail = m_tail->prev;
        if (m_tail != nullptr) m_tail->next = nullptr;
        else m_head = nullptr;
        delete old;
        --m_size;
    }

    T& front() { checkNotEmpty(); return m_head->data; }
    T& back()  { checkNotEmpty(); return m_tail->data; }
    const T& front() const { checkNotEmpty(); return m_head->data; }
    const T& back()  const { checkNotEmpty(); return m_tail->data; }

    // Read-only access to the nodes, for traversal.
    const Node* first() const { return m_head; }
    const Node* last()  const { return m_tail; }

    int size() const { return m_size; }
    bool isEmpty() const { return m_size == 0; }

    void clear() {
        Node* current = m_head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        m_head = m_tail = nullptr;
        m_size = 0;
    }

private:
    Node* m_head;
    Node* m_tail;
    int m_size;

    void checkNotEmpty() const {
        if (m_head == nullptr)
            throw std::out_of_range("LinkedList is empty");
    }

    void copyFrom(const LinkedList& other) {
        for (const Node* n = other.m_head; n != nullptr; n = n->next)
            pushBack(n->data);
    }
};

#endif // PRISM_LINKED_LIST_H
