#ifndef PRISM_HASH_TABLE_H
#define PRISM_HASH_TABLE_H

#include "DSA/DynamicArray.h"
#include <string>

// ---------------------------------------------------------------------------
// HashTable  -  separate chaining hash table.
//
// Maps a text KEY  ->  list of row indices where that key appears.
// (Numbers are stored by their text form, e.g. "23.5".)
//
//  - Each bucket is a singly linked list of nodes (the "chain").
//  - Hash function: polynomial rolling hash  h = h * 31 + c
//  - When loadFactor = size / capacity exceeds 0.75 the table grows to the
//    next PRIME number bigger than 2 * capacity (primes spread keys better).
//
//  - insert / find / remove : O(1) on average
//
// Used for: duplicate-row detection, exact-value lookup, hash search.
// ---------------------------------------------------------------------------
class HashTable {
public:
    explicit HashTable(int initialCapacity = 11);
    ~HashTable();

    // The table owns raw pointers, so copying is disabled to avoid double deletes.
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Adds rowIndex to the list of the given key (creates the key if new).
    void insert(const std::string& key, int rowIndex);

    // Returns the row list of a key, or nullptr if the key is not stored.
    const DynamicArray<int>* find(const std::string& key) const;

    // Same as find(), but also reports how many key comparisons were needed.
    const DynamicArray<int>* find(const std::string& key, int& comparisons) const;

    bool contains(const std::string& key) const;
    bool remove(const std::string& key);
    void clear();

    // Copies every stored key into 'out' (order is not meaningful).
    void keys(DynamicArray<std::string>& out) const;

    // ----- statistics (shown in the GUI) -----
    int size() const { return m_size; }            // number of distinct keys
    int capacity() const { return m_capacity; }    // number of buckets
    double loadFactor() const;
    int collisions() const { return m_collisions; }
    int rehashCount() const { return m_rehashCount; }
    int longestChain() const;

    static unsigned long long hashString(const std::string& key);

private:
    struct Node {
        std::string key;
        DynamicArray<int> rows;
        Node* next;
    };

    Node** m_buckets;     // array of chain heads
    int m_capacity;
    int m_size;
    int m_collisions;     // inserts that landed in a non-empty bucket
    int m_rehashCount;

    int bucketIndex(const std::string& key, int capacity) const;
    void rehash();
    void freeAllNodes();

    static bool isPrime(int n);
    static int nextPrime(int n);
};

#endif // PRISM_HASH_TABLE_H
