#ifndef PRISM_GRAPH_H
#define PRISM_GRAPH_H

#include "DSA/DynamicArray.h"
#include "DSA/LinkedList.h"
#include <string>

// ---------------------------------------------------------------------------
// Graph  -  undirected, weighted graph stored as an ADJACENCY LIST.
//
// In PRISM:  node  = a numeric column (e.g. "Temperature")
//            edge  = strong correlation between two columns (|r| >= threshold)
//            weight= the correlation coefficient r
//
//  - addNode / addEdge : O(1)
//  - BFS / DFS         : O(V + E)
// ---------------------------------------------------------------------------
class Graph {
public:
    struct Edge {
        int to = 0;
        double weight = 0.0;
    };

    int addNode(const std::string& label);                // returns the new node id
    void addEdge(int from, int to, double weight);        // undirected
    bool hasEdge(int from, int to) const;

    int nodeCount() const { return m_labels.size(); }
    int edgeCount() const { return m_edgeCount; }
    int degree(int node) const { return m_adjacency[node].size(); }
    const std::string& label(int node) const { return m_labels[node]; }
    int findNode(const std::string& label) const;         // -1 if missing
    const LinkedList<Edge>& neighbors(int node) const { return m_adjacency[node]; }

    // Traversals return the nodes in the order they were visited.
    DynamicArray<int> bfs(int start) const;
    DynamicArray<int> dfs(int start) const;

    // componentOf[node] = cluster number (0, 1, 2 ...). Returns the cluster count.
    int connectedComponents(DynamicArray<int>& componentOf) const;

    void clear();

private:
    DynamicArray<std::string> m_labels;
    DynamicArray<LinkedList<Edge>> m_adjacency;   // one list of edges per node
    int m_edgeCount = 0;

    void dfsRec(int node, DynamicArray<bool>& visited, DynamicArray<int>& order) const;
};

#endif // PRISM_GRAPH_H
