#ifndef PRISM_COLUMN_INDEX_H
#define PRISM_COLUMN_INDEX_H

#include "Data/Dataset.h"
#include "DSA/BST.h"
#include "DSA/HashTable.h"

// ---------------------------------------------------------------------------
// ColumnIndex - indexes ONE column two ways:
//
//   HashTable : exact lookups ("find rows where City == Karachi")  O(1) avg
//   BST       : ordered / range lookups ("Temperature between 20 and 30")
//               only for numeric columns                         O(log n) avg
//
// The BST is filled in RANDOM order. If we inserted an already sorted column
// (e.g. years 1990, 1991, ...) the tree would turn into a long "linked list"
// with O(n) height. Shuffling first keeps the expected height O(log n).
// ---------------------------------------------------------------------------
class ColumnIndex {
public:
    void build(const Dataset& data, int col);
    void clear();

    bool isBuilt() const { return m_column >= 0; }
    int column() const { return m_column; }
    bool hasTree() const { return !m_tree.isEmpty(); }

    // Exact match on the cell text (hash table).
    const DynamicArray<int>* findExact(const std::string& value) const;

    // Rows with low <= value <= high (BST).
    DynamicArray<int> findRange(double low, double high) const;

    const BST& tree() const { return m_tree; }
    const HashTable& hash() const { return m_hash; }

private:
    int m_column = -1;
    BST m_tree;
    HashTable m_hash;
};

#endif // PRISM_COLUMN_INDEX_H
