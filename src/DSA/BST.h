#ifndef PRISM_BST_H
#define PRISM_BST_H

#include "DSA/DynamicArray.h"

// ---------------------------------------------------------------------------
// BST  -  Binary Search Tree used as a column index.
//
// Key   = a numeric cell value (double)
// Value = list of row indices having that value (so duplicates are allowed)
//
// Rule: every key in the LEFT subtree is smaller, every key in the RIGHT
// subtree is bigger than the node's key.
//
//  - insert / search / remove : O(log n) on average, O(n) worst case
//  - in-order traversal       : O(n)   (gives keys in sorted order)
//  - range query [low, high]  : O(log n + k)
//
// search() can also record the PATH of visited keys so the GUI can animate it.
// ---------------------------------------------------------------------------
class BST {
public:
    struct Node {
        double key;
        DynamicArray<int> rows;
        Node* left;
        Node* right;
    };

    BST();
    ~BST();

    BST(const BST&) = delete;             // owns nodes -> no copying
    BST& operator=(const BST&) = delete;

    void insert(double key, int rowIndex);

    // Returns rows for the key, or nullptr if not found.
    const DynamicArray<int>* search(double key) const;
    // Same, but fills 'path' with every key visited on the way down.
    const DynamicArray<int>* search(double key, DynamicArray<double>& path) const;

    bool remove(double key);

    // All row indices whose key is inside [low, high], in ascending key order.
    void rangeQuery(double low, double high, DynamicArray<int>& outRows) const;

    // Keys in sorted order.
    void inOrder(DynamicArray<double>& outKeys) const;

    double minKey() const;
    double maxKey() const;

    int size() const { return m_size; }    // number of distinct keys (nodes)
    int height() const;
    bool isEmpty() const { return m_root == nullptr; }
    void clear();

    const Node* root() const { return m_root; }   // used by the tree drawing widget

private:
    Node* m_root;
    int m_size;

    // Recursive helpers
    Node* insertRec(Node* node, double key, int rowIndex);
    Node* removeRec(Node* node, double key, bool& removed);
    void rangeRec(const Node* node, double low, double high, DynamicArray<int>& out) const;
    void inOrderRec(const Node* node, DynamicArray<double>& out) const;
    int heightRec(const Node* node) const;
    void destroyRec(Node* node);
};

#endif // PRISM_BST_H
