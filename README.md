# PRISM – Scientific Data Analyzer

Data Structures (CL-2001) semester project, FAST NUCES Karachi.

PRISM is a C++17 desktop app that **imports, cleans, indexes, analyses, visualises and reports** on
scientific CSV datasets (weather, rainfall, population, earthquakes).
Every data structure and algorithm is written from scratch. The core does **not** use STL containers
(`std::vector`, `std::map`, …) or any statistics library. Qt is used **only** for the GUI.

---

## 1. Build & run

**Requirements:** Qt 6 with the MinGW kit (tested with Qt 6.8.3 + MinGW 13.1). CMake and Ninja come with Qt.

### Option A – one click
Double-click `build.bat`, or run it from a terminal in this folder. It builds everything, copies
the Qt DLLs next to the exe and runs the unit tests. Then start `build\PRISM.exe`.
(If Qt is not in `C:\Qt\6.8.3`, change the paths at the top of `build.bat`.)

### Option B – Qt Creator
`File → Open File or Project…` → choose `CMakeLists.txt` → select the *Desktop Qt 6 MinGW 64-bit* kit → Run.

### Executables produced
| File | What it is |
|---|---|
| `PRISM.exe` | the GUI application |
| `prism_tests.exe` | unit tests for every data structure and module (99 checks) |
| `prism_console.exe <file.csv>` | backend only, no GUI: prints stats and writes an HTML report |

---

## 2. Folder structure

```
PRISM/
├── CMakeLists.txt        build script (prism_core library + 3 executables)
├── build.bat             one-click Windows build
├── data/                 sample datasets (clean and "messy" versions)
├── tools/                generate_samples.py – recreates the sample CSV files
├── tests/                test_main.cpp (unit tests), console_demo.cpp
└── src/
    ├── DSA/              data structures           (backend)
    ├── Algorithms/       sorting, searching, statistics, correlation, Top-K
    ├── Data/             Dataset, CSV import, validation, cleaning, undo, history, batch queue
    ├── FileSystem/       .sda project save/load, CSV export, HTML report
    ├── Visualization/    chart widgets drawn with QPainter  (GUI)
    ├── UI/               the main window and all screens    (GUI)
    └── main.cpp
```

Every class has its own `.h` (declaration) and `.cpp` (implementation) file.
**Exception:** `DynamicArray`, `LinkedList`, `Stack`, `Queue` and `Heap` are **templates**
(they work with any type: `DynamicArray<int>`, `DynamicArray<double>`, …). C++ needs to see the full
template code wherever it is used, so templates must stay entirely in the header.
`HashTable`, `BST` and `Graph` are not templates, so they are split into `.h` + `.cpp`.

---

## 3. Where each data structure is used

| Structure | File | Used for |
|---|---|---|
| Dynamic Array | `DSA/DynamicArray.h` | storing every column and cell, all internal lists |
| Doubly Linked List | `DSA/LinkedList.h` | history log (shown newest-first by walking `prev`), graph adjacency lists |
| Stack | `DSA/Stack.h` | undo / redo (`Data/UndoManager`) |
| Circular Queue | `DSA/Queue.h` | batch analysis pipeline (`Data/BatchProcessor`), BFS |
| Binary Search Tree | `DSA/BST.h/.cpp` | column index, range queries, animated search in the Algorithm Lab |
| Heap | `DSA/Heap.h` | Top-K ranking, most-frequent text values |
| Hash Table | `DSA/HashTable.h/.cpp` | duplicate-row detection, exact lookup, hash search, grouping for bar charts |
| Graph + BFS/DFS | `DSA/Graph.h/.cpp` | correlation network, clusters of related columns |

| Algorithm | File |
|---|---|
| Bubble, Selection, Insertion, Merge, Quick, Heap sort (with counters) | `Algorithms/Sorting.h/.cpp` |
| Linear, Binary, Hash search | `Algorithms/Searching.h/.cpp` |
| Mean, median, mode, variance, std-dev, quartiles, percentiles, frequency distribution | `Algorithms/Statistics.h/.cpp` |
| Pearson correlation matrix, linear regression, graph building | `Algorithms/Correlation.h/.cpp` |
| Heap-based Top-K | `Algorithms/TopK.h/.cpp` |

## 4. OOP concepts used

* **Encapsulation** – every structure keeps its data `private` and exposes a small public interface.
* **Abstract classes + inheritance + polymorphism**
  * `Sorter` → `BubbleSorter`, `QuickSorter`, … (the Algorithm Lab loops over `Sorter*`).
    `Sorter::sort()` times the run and calls the virtual `sortImpl()` (Template Method pattern).
  * `BatchJob` → `ValidationJob`, `ColumnStatisticsJob`, `CorrelationJob`, `TopKJob` (stored in a `Queue<BatchJob*>`).
  * `ChartWidget` → `LineChart`, `BarChart`, `ScatterPlot`, `Histogram`. The base class draws the axes and each subclass draws only its data.
  * `BaseView` → every screen (`DashboardView`, `DatasetView`, …). `MainWindow` only knows `BaseView*`.
* **Templates** – `DynamicArray<T>`, `LinkedList<T>`, `Stack<T>`, `Queue<T>`, `Heap<T>`.
* **Rule of three** – deep copy constructor, assignment operator and destructor in the containers.
  Classes that own raw pointers but don't need copying (`HashTable`, `BST`, `Queue`) delete their copy operations.

---

## 5. Using the app

1. **Dashboard:** open a CSV (or click a sample, e.g. `weather_messy`). You see rows, columns, missing,
   invalid and duplicate counts. *Run full analysis* sends the batch jobs through the queue.
2. **Dataset:** the table colours missing cells red, invalid cells orange and duplicate rows purple.
   Clean with *Remove duplicate rows*, *Handle missing values* (mean / median / zero / remove rows) and
   *Fix invalid values*. Rename or remove columns. **Every change can be undone (Ctrl+Z / Ctrl+Y).**
   *Search & Index*: exact search uses the hash table; range search (from–to) uses the BST.
3. **Statistics:** all descriptive statistics, a histogram and a frequency table for one column, plus the correlation heat map.
4. **Charts:** line (optionally one line per group), bar (average per group), scatter (with trend line and r) and histogram.
5. **Compare:** *Load comparison dataset* (e.g. open `weather_2025.csv`, then compare with `weather_2024.csv`).
6. **Algorithm Lab:** sorting benchmark (try the different *input orders*), search comparison, Top-K,
   and the **BST visualizer** (animated search; try *Sorted order* to see the tree turn into a linked list).
7. **Relationship Graph:** columns are nodes, strong correlations are edges. BFS and DFS are animated step by step.
8. **File menu:** save/open the session as a `.sda` project, export the cleaned CSV, export an HTML report.

## 6. The `.sda` project format

A plain text file with tab-separated fields (see `FileSystem/ProjectFile.h`):

```
PRISM-SDA   1
NAME        weather
SOURCE      C:/data/weather.csv
COLUMNS     8
COLUMN      T   Date          (N = numeric, T = text)
...
ROWS        1095
<one line per row>
HISTORY     5
<timestamp> <action>
END
```

## 7. Sample data

`data/` contains synthetic but realistic datasets. The `*_messy.csv` files contain missing values
(`""`, `NA`, `null`, `?`), invalid values (`#96.8`, `23.5C`, `error`) and about 4% duplicate rows.
Regenerate them with `python tools/generate_samples.py`.

## 8. Division of work (from the proposal)

| Member | Main files |
|---|---|
| Abdullah Aamir Farooqui (25K-0868) | `DSA/DynamicArray`, `Data/CSVReader`, `Data/DataValidator`, `Data/DataCleaner`, `Algorithms/Statistics`, `Algorithms/Correlation`, `DSA/Graph`, `Data/UndoManager` |
| Tahir Hussain (25K-0532) | `DSA/HashTable`, `DSA/BST`, `DSA/Heap`, `Algorithms/TopK`, `Algorithms/Sorting`, `Algorithms/Searching`, `FileSystem/*` |
| Ali Shanawar (25K-0564) | `DSA/Queue`, `DSA/LinkedList`, `Data/BatchProcessor`, `Data/HistoryLog`, `UI/*`, `Visualization/*` |
