#include "DSA/BST.h"
#include <stdexcept>

BST::BST() : m_root(nullptr), m_size(0) {}

BST::~BST() {
    clear();
}

void BST::clear() {
    destroyRec(m_root);
    m_root = nullptr;
    m_size = 0;
}

void BST::destroyRec(Node* node) {
    if (node == nullptr)
        return;
    destroyRec(node->left);
    destroyRec(node->right);
    delete node;
}

// ------------------------------- insert -----------------------------------

void BST::insert(double key, int rowIndex) {
    m_root = insertRec(m_root, key, rowIndex);
}

BST::Node* BST::insertRec(Node* node, double key, int rowIndex) {
    if (node == nullptr) {               // empty spot found -> create node
        Node* created = new Node;
        created->key = key;
        created->rows.pushBack(rowIndex);
        created->left = created->right = nullptr;
        ++m_size;
        return created;
    }
    if (key < node->key)
        node->left = insertRec(node->left, key, rowIndex);
    else if (key > node->key)
        node->right = insertRec(node->right, key, rowIndex);
    else
        node->rows.pushBack(rowIndex);   // same key -> remember another row
    return node;
}

// ------------------------------- search -----------------------------------

const DynamicArray<int>* BST::search(double key) const {
    DynamicArray<double> ignoredPath;
    return search(key, ignoredPath);
}

const DynamicArray<int>* BST::search(double key, DynamicArray<double>& path) const {
    path.clear();
    const Node* current = m_root;
    while (current != nullptr) {
        path.pushBack(current->key);
        if (key == current->key)
            return &current->rows;
        current = (key < current->key) ? current->left : current->right;
    }
    return nullptr;
}

// ------------------------------- remove -----------------------------------

bool BST::remove(double key) {
    bool removed = false;
    m_root = removeRec(m_root, key, removed);
    if (removed)
        --m_size;
    return removed;
}

BST::Node* BST::removeRec(Node* node, double key, bool& removed) {
    if (node == nullptr)
        return nullptr;

    if (key < node->key) {
        node->left = removeRec(node->left, key, removed);
    } else if (key > node->key) {
        node->right = removeRec(node->right, key, removed);
    } else {
        removed = true;
        // Case 1 & 2: zero or one child -> replace node by its child
        if (node->left == nullptr) {
            Node* child = node->right;
            delete node;
            return child;
        }
        if (node->right == nullptr) {
            Node* child = node->left;
            delete node;
            return child;
        }
        // Case 3: two children -> copy the in-order successor (smallest in
        // the right subtree) into this node, then delete the successor.
        Node* successor = node->right;
        while (successor->left != nullptr)
            successor = successor->left;
        node->key = successor->key;
        node->rows = successor->rows;
        bool dummy = false;
        node->right = removeRec(node->right, successor->key, dummy);
    }
    return node;
}

// ---------------------------- range / order -------------------------------

void BST::rangeQuery(double low, double high, DynamicArray<int>& outRows) const {
    outRows.clear();
    rangeRec(m_root, low, high, outRows);
}

void BST::rangeRec(const Node* node, double low, double high, DynamicArray<int>& out) const {
    if (node == nullptr)
        return;
    if (low < node->key)                       // left side may contain matches
        rangeRec(node->left, low, high, out);
    if (node->key >= low && node->key <= high)
        for (int i = 0; i < node->rows.size(); ++i)
            out.pushBack(node->rows[i]);
    if (high > node->key)                      // right side may contain matches
        rangeRec(node->right, low, high, out);
}

void BST::inOrder(DynamicArray<double>& outKeys) const {
    outKeys.clear();
    inOrderRec(m_root, outKeys);
}

void BST::inOrderRec(const Node* node, DynamicArray<double>& out) const {
    if (node == nullptr)
        return;
    inOrderRec(node->left, out);
    out.pushBack(node->key);
    inOrderRec(node->right, out);
}

double BST::minKey() const {
    if (m_root == nullptr)
        throw std::out_of_range("BST is empty");
    const Node* n = m_root;
    while (n->left != nullptr)
        n = n->left;
    return n->key;
}

double BST::maxKey() const {
    if (m_root == nullptr)
        throw std::out_of_range("BST is empty");
    const Node* n = m_root;
    while (n->right != nullptr)
        n = n->right;
    return n->key;
}

int BST::height() const {
    return heightRec(m_root);
}

int BST::heightRec(const Node* node) const {
    if (node == nullptr)
        return 0;
    int left = heightRec(node->left);
    int right = heightRec(node->right);
    return 1 + (left > right ? left : right);
}
