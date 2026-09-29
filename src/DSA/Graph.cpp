#include "DSA/Graph.h"
#include "DSA/Queue.h"
#include <stdexcept>

int Graph::addNode(const std::string& label) {
    m_labels.pushBack(label);
    m_adjacency.pushBack(LinkedList<Edge>());
    return m_labels.size() - 1;
}

void Graph::addEdge(int from, int to, double weight) {
    if (from < 0 || to < 0 || from >= nodeCount() || to >= nodeCount())
        throw std::out_of_range("Graph::addEdge invalid node");
    if (from == to || hasEdge(from, to))
        return;

    Edge forward;
    forward.to = to;
    forward.weight = weight;
    m_adjacency[from].pushBack(forward);

    Edge backward;                 // undirected -> store both directions
    backward.to = from;
    backward.weight = weight;
    m_adjacency[to].pushBack(backward);

    ++m_edgeCount;
}

bool Graph::hasEdge(int from, int to) const {
    for (const LinkedList<Edge>::Node* n = m_adjacency[from].first(); n != nullptr; n = n->next)
        if (n->data.to == to)
            return true;
    return false;
}

int Graph::findNode(const std::string& label) const {
    return m_labels.indexOf(label);
}

// Breadth-first search: visit all neighbours first, then their neighbours...
DynamicArray<int> Graph::bfs(int start) const {
    DynamicArray<int> order;
    if (start < 0 || start >= nodeCount())
        return order;

    DynamicArray<bool> visited;
    visited.resize(nodeCount(), false);

    Queue<int> queue;
    queue.enqueue(start);
    visited[start] = true;

    while (!queue.isEmpty()) {
        int current = queue.dequeue();
        order.pushBack(current);
        for (const LinkedList<Edge>::Node* n = m_adjacency[current].first(); n != nullptr; n = n->next) {
            int next = n->data.to;
            if (!visited[next]) {
                visited[next] = true;
                queue.enqueue(next);
            }
        }
    }
    return order;
}

// Depth-first search: go as deep as possible before backtracking (recursive).
DynamicArray<int> Graph::dfs(int start) const {
    DynamicArray<int> order;
    if (start < 0 || start >= nodeCount())
        return order;
    DynamicArray<bool> visited;
    visited.resize(nodeCount(), false);
    dfsRec(start, visited, order);
    return order;
}

void Graph::dfsRec(int node, DynamicArray<bool>& visited, DynamicArray<int>& order) const {
    visited[node] = true;
    order.pushBack(node);
    for (const LinkedList<Edge>::Node* n = m_adjacency[node].first(); n != nullptr; n = n->next)
        if (!visited[n->data.to])
            dfsRec(n->data.to, visited, order);
}

// Runs a DFS from every unvisited node; each DFS discovers one cluster.
int Graph::connectedComponents(DynamicArray<int>& componentOf) const {
    componentOf.clear();
    componentOf.resize(nodeCount(), -1);
    DynamicArray<bool> visited;
    visited.resize(nodeCount(), false);

    int components = 0;
    for (int node = 0; node < nodeCount(); ++node) {
        if (visited[node])
            continue;
        DynamicArray<int> members;
        dfsRec(node, visited, members);
        for (int i = 0; i < members.size(); ++i)
            componentOf[members[i]] = components;
        ++components;
    }
    return components;
}

void Graph::clear() {
    m_labels.clear();
    m_adjacency.clear();
    m_edgeCount = 0;
}
