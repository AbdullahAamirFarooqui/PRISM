#include "DSA/HashTable.h"

HashTable::HashTable(int initialCapacity)
    : m_buckets(nullptr), m_capacity(nextPrime(initialCapacity < 3 ? 3 : initialCapacity)),
      m_size(0), m_collisions(0), m_rehashCount(0) {
    m_buckets = new Node*[m_capacity];
    for (int i = 0; i < m_capacity; ++i)
        m_buckets[i] = nullptr;
}

HashTable::~HashTable() {
    freeAllNodes();
    delete[] m_buckets;
}

// Polynomial rolling hash: treats the string like a number in base 31.
unsigned long long HashTable::hashString(const std::string& key) {
    unsigned long long hash = 0;
    for (char c : key)
        hash = hash * 31ULL + static_cast<unsigned char>(c);
    // Small "bit mix" at the end so similar keys spread out more.
    hash ^= (hash >> 33);
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= (hash >> 33);
    return hash;
}

int HashTable::bucketIndex(const std::string& key, int capacity) const {
    return static_cast<int>(hashString(key) % static_cast<unsigned long long>(capacity));
}

void HashTable::insert(const std::string& key, int rowIndex) {
    int index = bucketIndex(key, m_capacity);

    // 1) key already exists -> just add the row
    for (Node* n = m_buckets[index]; n != nullptr; n = n->next) {
        if (n->key == key) {
            n->rows.pushBack(rowIndex);
            return;
        }
    }

    // 2) new key -> insert a node at the head of the chain
    if (m_buckets[index] != nullptr)
        ++m_collisions;

    Node* node = new Node;
    node->key = key;
    node->rows.pushBack(rowIndex);
    node->next = m_buckets[index];
    m_buckets[index] = node;
    ++m_size;

    if (loadFactor() > 0.75)
        rehash();
}

const DynamicArray<int>* HashTable::find(const std::string& key) const {
    int comparisons = 0;
    return find(key, comparisons);
}

const DynamicArray<int>* HashTable::find(const std::string& key, int& comparisons) const {
    comparisons = 0;
    int index = bucketIndex(key, m_capacity);
    for (Node* n = m_buckets[index]; n != nullptr; n = n->next) {
        ++comparisons;
        if (n->key == key)
            return &n->rows;
    }
    return nullptr;
}

bool HashTable::contains(const std::string& key) const {
    return find(key) != nullptr;
}

bool HashTable::remove(const std::string& key) {
    int index = bucketIndex(key, m_capacity);
    Node* previous = nullptr;
    Node* current = m_buckets[index];
    while (current != nullptr) {
        if (current->key == key) {
            if (previous == nullptr) m_buckets[index] = current->next;
            else previous->next = current->next;
            delete current;
            --m_size;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

void HashTable::clear() {
    freeAllNodes();
    m_size = 0;
    m_collisions = 0;
}

void HashTable::keys(DynamicArray<std::string>& out) const {
    out.clear();
    out.reserve(m_size);
    for (int i = 0; i < m_capacity; ++i)
        for (Node* n = m_buckets[i]; n != nullptr; n = n->next)
            out.pushBack(n->key);
}

double HashTable::loadFactor() const {
    return static_cast<double>(m_size) / m_capacity;
}

int HashTable::longestChain() const {
    int longest = 0;
    for (int i = 0; i < m_capacity; ++i) {
        int length = 0;
        for (Node* n = m_buckets[i]; n != nullptr; n = n->next)
            ++length;
        if (length > longest)
            longest = length;
    }
    return longest;
}

// Moves every node into a new, bigger bucket array.
// Nodes are re-linked (not copied), so this is cheap.
void HashTable::rehash() {
    int newCapacity = nextPrime(m_capacity * 2 + 1);
    Node** newBuckets = new Node*[newCapacity];
    for (int i = 0; i < newCapacity; ++i)
        newBuckets[i] = nullptr;

    for (int i = 0; i < m_capacity; ++i) {
        Node* n = m_buckets[i];
        while (n != nullptr) {
            Node* next = n->next;
            int index = bucketIndex(n->key, newCapacity);
            n->next = newBuckets[index];
            newBuckets[index] = n;
            n = next;
        }
    }

    delete[] m_buckets;
    m_buckets = newBuckets;
    m_capacity = newCapacity;
    ++m_rehashCount;
}

void HashTable::freeAllNodes() {
    for (int i = 0; i < m_capacity; ++i) {
        Node* n = m_buckets[i];
        while (n != nullptr) {
            Node* next = n->next;
            delete n;
            n = next;
        }
        m_buckets[i] = nullptr;
    }
}

bool HashTable::isPrime(int n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (int d = 3; d * d <= n; d += 2)
        if (n % d == 0)
            return false;
    return true;
}

int HashTable::nextPrime(int n) {
    while (!isPrime(n))
        ++n;
    return n;
}
