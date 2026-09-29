// ---------------------------------------------------------------------------
// PRISM unit tests - one small test function per data structure / module.
// No test framework needed: CHECK() prints a line and counts failures.
// Run:  prism_tests.exe   (from the build folder)
// ---------------------------------------------------------------------------
#include "DSA/DynamicArray.h"
#include "DSA/LinkedList.h"
#include "DSA/Stack.h"
#include "DSA/Queue.h"
#include "DSA/Heap.h"
#include "DSA/HashTable.h"
#include "DSA/BST.h"
#include "DSA/Graph.h"
#include "Algorithms/Sorting.h"
#include "Algorithms/Searching.h"
#include "Algorithms/Statistics.h"
#include "Algorithms/Correlation.h"
#include "Algorithms/TopK.h"
#include "Data/Dataset.h"
#include "Data/DataValidator.h"
#include "Data/DataCleaner.h"
#include "Data/Workspace.h"
#include "FileSystem/SaveManager.h"
#include "FileSystem/LoadManager.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

static int g_passed = 0;
static int g_failed = 0;

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (condition) { ++g_passed; }                                          \
        else { ++g_failed; std::cout << "  FAILED: " #condition                 \
                                     << "  (line " << __LINE__ << ")\n"; }      \
    } while (0)

static bool nearlyEqual(double a, double b, double eps = 1e-6) { return std::fabs(a - b) < eps; }

static void testDynamicArray() {
    std::cout << "DynamicArray\n";
    DynamicArray<int> a;
    for (int i = 0; i < 100; ++i) a.pushBack(i);
    CHECK(a.size() == 100);
    CHECK(a[57] == 57);
    a.insertAt(0, -1);
    CHECK(a[0] == -1 && a.size() == 101);
    a.removeAt(0);
    CHECK(a[0] == 0);
    DynamicArray<int> copy = a;          // deep copy
    copy[0] = 999;
    CHECK(a[0] == 0);
    CHECK(a.indexOf(42) == 42);
    bool threw = false;
    try { a.at(1000); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
}

static void testLinkedList() {
    std::cout << "LinkedList\n";
    LinkedList<std::string> list;
    list.pushBack("b");
    list.pushBack("c");
    list.pushFront("a");
    CHECK(list.size() == 3);
    CHECK(list.front() == "a" && list.back() == "c");
    std::string joined;
    for (const LinkedList<std::string>::Node* n = list.first(); n; n = n->next) joined += n->data;
    CHECK(joined == "abc");
    std::string reversed;
    for (const LinkedList<std::string>::Node* n = list.last(); n; n = n->prev) reversed += n->data;
    CHECK(reversed == "cba");
    list.popFront();
    list.popBack();
    CHECK(list.size() == 1 && list.front() == "b");
}

static void testStackAndQueue() {
    std::cout << "Stack & Queue\n";
    Stack<int> s;
    s.push(1); s.push(2); s.push(3);
    CHECK(s.pop() == 3);
    CHECK(s.top() == 2 && s.size() == 2);

    Queue<int> q(2);                     // tiny capacity to force wrap-around + growth
    for (int i = 1; i <= 5; ++i) q.enqueue(i);
    CHECK(q.dequeue() == 1);
    CHECK(q.dequeue() == 2);
    q.enqueue(6);
    q.enqueue(7);
    CHECK(q.size() == 5);
    CHECK(q.front() == 3);
    int last = 0;
    while (!q.isEmpty()) last = q.dequeue();
    CHECK(last == 7);
}

static void testHeap() {
    std::cout << "Heap\n";
    Heap<int> minHeap(true);
    int values[] = {5, 3, 8, 1, 9, 2};
    for (int v : values) minHeap.push(v);
    CHECK(minHeap.pop() == 1);
    CHECK(minHeap.pop() == 2);

    Heap<int> maxHeap(false);
    DynamicArray<int> arr;
    for (int v : values) arr.pushBack(v);
    maxHeap.buildFrom(arr);
    CHECK(maxHeap.pop() == 9);
    CHECK(maxHeap.pop() == 8);
}

static void testHashTable() {
    std::cout << "HashTable\n";
    HashTable table(3);
    for (int i = 0; i < 1000; ++i) table.insert("key" + std::to_string(i), i);
    table.insert("key5", 5000);
    CHECK(table.size() == 1000);
    CHECK(table.loadFactor() <= 0.75);
    CHECK(table.rehashCount() > 0);
    const DynamicArray<int>* rows = table.find("key5");
    CHECK(rows != nullptr && rows->size() == 2);
    CHECK(table.find("missing") == nullptr);
    CHECK(table.remove("key7"));
    CHECK(!table.contains("key7"));
}

static void testBST() {
    std::cout << "BST\n";
    BST tree;
    double keys[] = {50, 30, 70, 20, 40, 60, 80, 30};
    for (int i = 0; i < 8; ++i) tree.insert(keys[i], i);
    CHECK(tree.size() == 7);                       // 30 is stored once with 2 rows
    CHECK(tree.search(30)->size() == 2);
    DynamicArray<double> path;
    tree.search(60, path);
    CHECK(path.size() == 3 && path[0] == 50 && path[1] == 70 && path[2] == 60);
    DynamicArray<double> sorted;
    tree.inOrder(sorted);
    CHECK(Sorting::isSorted(sorted));
    DynamicArray<int> range;
    tree.rangeQuery(35, 65, range);
    CHECK(range.size() == 3);                      // 40, 50, 60
    CHECK(tree.remove(50));                        // node with two children
    CHECK(tree.search(50) == nullptr && tree.size() == 6);
    tree.inOrder(sorted);
    CHECK(Sorting::isSorted(sorted));
    CHECK(tree.minKey() == 20 && tree.maxKey() == 80);
}

static void testGraph() {
    std::cout << "Graph\n";
    Graph g;
    for (int i = 0; i < 6; ++i) g.addNode("N" + std::to_string(i));
    g.addEdge(0, 1, 0.9);
    g.addEdge(1, 2, 0.8);
    g.addEdge(0, 3, 0.7);
    g.addEdge(4, 5, -0.9);
    CHECK(g.edgeCount() == 4);
    DynamicArray<int> bfs = g.bfs(0);
    CHECK(bfs.size() == 4 && bfs[0] == 0 && bfs[1] == 1 && bfs[2] == 3);
    DynamicArray<int> dfs = g.dfs(0);
    CHECK(dfs.size() == 4 && dfs[1] == 1 && dfs[2] == 2);
    DynamicArray<int> comp;
    CHECK(g.connectedComponents(comp) == 2);
    CHECK(comp[4] == comp[5] && comp[0] != comp[4]);
}

static void testSorting() {
    std::cout << "Sorting\n";
    DynamicArray<Sorter*> sorters = Sorting::createAll();
    for (int s = 0; s < sorters.size(); ++s) {
        DynamicArray<double> data;
        std::srand(7);
        for (int i = 0; i < 500; ++i) data.pushBack(std::rand() % 1000);
        SortStats stats = sorters[s]->sort(data);
        CHECK(Sorting::isSorted(data));
        CHECK(stats.comparisons > 0);
        // already sorted input must also work (quick sort worst case check)
        sorters[s]->sort(data);
        CHECK(Sorting::isSorted(data));
    }
    Sorting::destroyAll(sorters);
}

static void testSearching() {
    std::cout << "Searching\n";
    DynamicArray<double> data;
    for (int i = 0; i < 1000; ++i) data.pushBack(i * 2);
    SearchResult lin = Searching::linearSearch(data, 500);
    SearchResult bin = Searching::binarySearch(data, 500);
    CHECK(lin.found && lin.index == 250);
    CHECK(bin.found && bin.index == 250);
    CHECK(bin.comparisons < lin.comparisons);
    CHECK(!Searching::binarySearch(data, 501).found);
    HashTable table;
    table.insert("500", 250);
    CHECK(Searching::hashSearch(table, "500").found);
}

static void testStatistics() {
    std::cout << "Statistics\n";
    DynamicArray<double> v;
    double values[] = {2, 4, 4, 4, 5, 5, 7, 9};
    for (double x : values) v.pushBack(x);
    ColumnSummary s = Statistics::summarize(v);
    CHECK(nearlyEqual(s.mean, 5.0));
    CHECK(nearlyEqual(s.variance, 4.0));
    CHECK(nearlyEqual(s.stdDev, 2.0));
    CHECK(nearlyEqual(s.median, 4.5));
    CHECK(nearlyEqual(s.mode, 4.0) && s.modeFrequency == 3);
    CHECK(nearlyEqual(s.range, 7.0));
    DynamicArray<FrequencyBin> bins = Statistics::frequencyDistribution(v, 7);
    int total = 0;
    for (int i = 0; i < bins.size(); ++i) total += bins[i].count;
    CHECK(total == 8);
}

static void testCorrelationAndTopK() {
    std::cout << "Correlation & Top-K\n";
    DynamicArray<double> x, y, z;
    for (int i = 0; i < 50; ++i) {
        x.pushBack(i);
        y.pushBack(3 * i + 2);
        z.pushBack(-i);
    }
    CHECK(nearlyEqual(Correlation::pearson(x, y), 1.0));
    CHECK(nearlyEqual(Correlation::pearson(x, z), -1.0));
    double slope, intercept;
    CHECK(Correlation::linearFit(x, y, slope, intercept));
    CHECK(nearlyEqual(slope, 3.0) && nearlyEqual(intercept, 2.0));

    DynamicArray<RankedItem> top = TopK::largest(y, 3);
    CHECK(top.size() == 3 && top[0].value == 149 && top[2].value == 143);
}

static Dataset makeMessyDataset() {
    Dataset d;
    d.addColumn("Temp");
    d.addColumn("City");
    const char* rows[][2] = {{"20", "A"}, {"22", "B"}, {"NA", "A"}, {"abc", "C"},
                             {"20", "A"}, {"1,200", "B"}, {"24", ""}, {"21", "C"}, {"23", "B"}};
    for (auto& r : rows) {
        DynamicArray<std::string> cells;
        cells.pushBack(r[0]);
        cells.pushBack(r[1]);
        d.addRow(cells);
    }
    DataValidator::inferTypes(d);
    return d;
}

static void testValidatorAndCleaner() {
    std::cout << "DataValidator & DataCleaner\n";
    Dataset d = makeMessyDataset();
    CHECK(d.isNumeric(0) && !d.isNumeric(1));
    ValidationReport report = DataValidator::validate(d);
    CHECK(report.totalMissing == 2);          // "NA" and ""
    CHECK(report.totalInvalid == 2);          // "abc" and "1,200"
    CHECK(report.duplicateRows == 1);         // row 4 repeats row 0

    CHECK(DataCleaner::removeDuplicates(d) == 1);
    CHECK(d.rowCount() == 8);
    DataCleaner::fixInvalid(d, 0, MissingStrategy::Median);
    CHECK(d.cell(4, 0) == "1200");            // repaired
    CHECK(DataValidator::validate(d).columnReports[0].invalid == 0);
    DataCleaner::handleMissing(d, -1, MissingStrategy::Mean);
    CHECK(DataValidator::validate(d).totalMissing == 0);
}

static void testUndoAndProjectFile() {
    std::cout << "Undo/Redo & .sda files\n";
    Workspace ws;
    ws.data() = makeMessyDataset();
    ws.beginChange("remove duplicates");
    DataCleaner::removeDuplicates(ws.data());
    ws.endChange("Removed duplicates");
    CHECK(ws.data().rowCount() == 8);
    std::string what;
    CHECK(ws.undo(what) && what == "remove duplicates");
    CHECK(ws.data().rowCount() == 9);
    CHECK(ws.redo(what));
    CHECK(ws.data().rowCount() == 8);

    std::string error;
    ws.data().setCell(0, 1, "tab\there");     // make sure escaping works
    CHECK(ws.saveProject("prism_test_project.sda", error));
    Workspace other;
    CHECK(other.openProject("prism_test_project.sda", error));
    CHECK(other.data().rowCount() == 8 && other.data().columnCount() == 2);
    CHECK(other.data().cell(0, 1) == "tab\there");
    CHECK(other.data().isNumeric(0));
    CHECK(other.history().size() >= 2);
    std::remove("prism_test_project.sda");
}

int main() {
    std::cout << "===== PRISM unit tests =====\n";
    testDynamicArray();
    testLinkedList();
    testStackAndQueue();
    testHeap();
    testHashTable();
    testBST();
    testGraph();
    testSorting();
    testSearching();
    testStatistics();
    testCorrelationAndTopK();
    testValidatorAndCleaner();
    testUndoAndProjectFile();
    std::cout << "============================\n"
              << "Passed: " << g_passed << "   Failed: " << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
