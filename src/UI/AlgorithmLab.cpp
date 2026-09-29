#include "UI/AlgorithmLab.h"
#include "UI/Style.h"
#include "Algorithms/Searching.h"
#include "Algorithms/TopK.h"
#include "DSA/HashTable.h"
#include "Visualization/BarChart.h"
#include "Visualization/BSTWidget.h"
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QTimer>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QRandomGenerator>
#include <QLocale>
#include <cmath>

static const int QUADRATIC_LIMIT = 20000;   // O(n^2) sorts are skipped above this size

static QTableWidget* makeTable(const QStringList& headers) {
    QTableWidget* table = new QTableWidget;
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    return table;
}

static QTableWidgetItem* numberItem(const QString& text) {
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return item;
}

AlgorithmLab::AlgorithmLab(Workspace& workspace, QWidget* parent) : BaseView(workspace, parent) {
    m_sorters = Sorting::createAll();

    QTabWidget* tabs = new QTabWidget;
    tabs->addTab(buildSortTab(), "Sorting benchmark");
    tabs->addTab(buildSearchTab(), "Searching");
    tabs->addTab(buildTopKTab(), "Top-K (heap)");
    tabs->addTab(buildTreeTab(), "BST visualizer");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabs);

    m_animation = new QTimer(this);
    m_animation->setInterval(550);
    connect(m_animation, &QTimer::timeout, this, &AlgorithmLab::animationStep);
}

AlgorithmLab::~AlgorithmLab() {
    Sorting::destroyAll(m_sorters);
}

void AlgorithmLab::refresh() {
    fillColumnCombo(m_sortColumn, true);
    fillColumnCombo(m_searchColumn, true);
    fillColumnCombo(m_topColumn, true);
    fillColumnCombo(m_treeColumn, true);
    int rows = qMax(10, m_workspace.data().rowCount());
    m_sortSize->setMaximum(rows);
    if (m_sortSize->value() > rows)
        m_sortSize->setValue(rows);
}

// First maxCount valid numbers of a column (in dataset order).
DynamicArray<double> AlgorithmLab::sampleColumn(int col, int maxCount) const {
    DynamicArray<double> values;
    const Dataset& data = m_workspace.data();
    double v;
    for (int r = 0; r < data.rowCount() && values.size() < maxCount; ++r)
        if (data.numberAt(r, col, v))
            values.pushBack(v);
    return values;
}

// =============================== SORTING ===================================

QWidget* AlgorithmLab::buildSortTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);

    QHBoxLayout* controls = new QHBoxLayout;
    m_sortColumn = new QComboBox;
    m_sortColumn->setMinimumWidth(160);
    m_sortSize = new QSpinBox;
    m_sortSize->setRange(10, 1000000);
    m_sortSize->setValue(2000);
    m_sortSize->setSingleStep(500);
    m_sortOrder = new QComboBox;
    m_sortOrder->addItems({"Dataset order", "Random shuffle", "Already sorted", "Reverse sorted"});
    QPushButton* run = new QPushButton("Run benchmark");
    connect(run, &QPushButton::clicked, this, &AlgorithmLab::runSortBenchmark);
    controls->addWidget(new QLabel("Column:"));
    controls->addWidget(m_sortColumn);
    controls->addWidget(new QLabel("Elements (n):"));
    controls->addWidget(m_sortSize);
    controls->addWidget(new QLabel("Input order:"));
    controls->addWidget(m_sortOrder);
    controls->addWidget(run);
    controls->addStretch();
    layout->addLayout(controls);

    m_sortNote = Style::makeHint("Every algorithm sorts its own copy of the same input. "
                                 "Try 'Already sorted' to see Bubble/Insertion sort become fast.");
    layout->addWidget(m_sortNote);

    QSplitter* splitter = new QSplitter(Qt::Vertical);
    m_sortTable = makeTable({"Algorithm", "Complexity", "n", "Comparisons", "Swaps / moves", "Time (ms)", "Sorted?"});
    m_sortTable->setMinimumHeight(245);
    splitter->addWidget(m_sortTable);

    QWidget* chartBox = new QWidget;
    QVBoxLayout* chartLayout = new QVBoxLayout(chartBox);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* metricRow = new QHBoxLayout;
    m_sortMetric = new QComboBox;
    m_sortMetric->addItems({"Time (ms)", "Comparisons", "Swaps / moves"});
    connect(m_sortMetric, &QComboBox::currentIndexChanged, this, &AlgorithmLab::updateSortChart);
    metricRow->addWidget(new QLabel("Chart shows:"));
    metricRow->addWidget(m_sortMetric);
    metricRow->addStretch();
    chartLayout->addLayout(metricRow);
    m_sortChart = new BarChart;
    chartLayout->addWidget(m_sortChart);
    splitter->addWidget(chartBox);
    splitter->setStretchFactor(1, 2);
    layout->addWidget(splitter, 1);
    return page;
}

void AlgorithmLab::runSortBenchmark() {
    int col = selectedColumn(m_sortColumn);
    if (col < 0) return;
    DynamicArray<double> input = sampleColumn(col, m_sortSize->value());
    int n = input.size();
    if (n == 0) return;

    // Prepare the input order
    switch (m_sortOrder->currentIndex()) {
    case 1:
        for (int i = n - 1; i > 0; --i)
            input.swap(i, QRandomGenerator::global()->bounded(i + 1));
        break;
    case 2:
        Sorting::sortAscending(input);
        break;
    case 3:
        Sorting::sortAscending(input);
        for (int i = 0; i < n / 2; ++i)
            input.swap(i, n - 1 - i);
        break;
    default:
        break;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_lastResults.clear();
    m_sortTable->setRowCount(0);
    for (int s = 0; s < m_sorters.size(); ++s) {
        Sorter* sorter = m_sorters[s];
        int row = m_sortTable->rowCount();
        m_sortTable->insertRow(row);
        m_sortTable->setItem(row, 0, new QTableWidgetItem(Style::toQ(sorter->name())));
        m_sortTable->setItem(row, 1, new QTableWidgetItem(Style::toQ(sorter->complexity())));

        bool quadratic = sorter->complexity() == "O(n^2)";
        if (quadratic && n > QUADRATIC_LIMIT) {
            m_sortTable->setItem(row, 2, numberItem(QString::number(n)));
            m_sortTable->setItem(row, 3, new QTableWidgetItem("skipped (n too large for O(n^2))"));
            continue;
        }
        DynamicArray<double> copy = input;
        SortStats stats = sorter->sort(copy);          // virtual sortImpl() inside
        m_lastResults.append(stats);

        m_sortTable->setItem(row, 2, numberItem(QString::number(stats.elements)));
        m_sortTable->setItem(row, 3, numberItem(QLocale().toString(stats.comparisons)));
        m_sortTable->setItem(row, 4, numberItem(QLocale().toString(stats.swaps)));
        m_sortTable->setItem(row, 5, numberItem(QString::number(stats.milliseconds, 'f', 3)));
        m_sortTable->setItem(row, 6, new QTableWidgetItem(Sorting::isSorted(copy) ? "yes" : "NO"));
        QApplication::processEvents();                 // keep the window responsive
    }
    QApplication::restoreOverrideCursor();

    m_workspace.log("Sorting benchmark on '" + m_workspace.data().columnName(col) + "' (n=" +
                    std::to_string(n) + ", " + Style::toStd(m_sortOrder->currentText()) + ")");
    emit dataModified();
    updateSortChart();
}

void AlgorithmLab::updateSortChart() {
    QStringList labels;
    QVector<double> values;
    int metric = m_sortMetric->currentIndex();
    for (const SortStats& s : m_lastResults) {
        labels << Style::toQ(s.algorithm);
        values << (metric == 0 ? s.milliseconds : metric == 1 ? double(s.comparisons) : double(s.swaps));
    }
    m_sortChart->setTitle(m_sortMetric->currentText() + " per algorithm");
    m_sortChart->setAxisLabels("algorithm", m_sortMetric->currentText());
    m_sortChart->setBars(labels, values);
}

// ============================== SEARCHING ==================================

QWidget* AlgorithmLab::buildSearchTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);

    QHBoxLayout* controls = new QHBoxLayout;
    m_searchColumn = new QComboBox;
    m_searchColumn->setMinimumWidth(160);
    m_searchTarget = new QDoubleSpinBox;
    m_searchTarget->setRange(-1e12, 1e12);
    m_searchTarget->setDecimals(4);
    QPushButton* random = new QPushButton("Pick existing value");
    random->setProperty("secondary", true);
    QPushButton* run = new QPushButton("Search");
    connect(random, &QPushButton::clicked, this, &AlgorithmLab::pickRandomTarget);
    connect(run, &QPushButton::clicked, this, &AlgorithmLab::runSearch);
    controls->addWidget(new QLabel("Column:"));
    controls->addWidget(m_searchColumn);
    controls->addWidget(new QLabel("Value:"));
    controls->addWidget(m_searchTarget);
    controls->addWidget(random);
    controls->addWidget(run);
    controls->addStretch();
    layout->addLayout(controls);

    m_searchTable = makeTable({"Algorithm", "Works on", "Found?", "Position", "Comparisons", "Time (µs)"});
    layout->addWidget(m_searchTable);
    m_searchNote = Style::makeHint(
        "Linear search scans the column in dataset order. Binary search needs a sorted copy "
        "(sorting time not included). Hash search uses the custom hash table built from the column.");
    layout->addWidget(m_searchNote);
    layout->addStretch();
    return page;
}

void AlgorithmLab::pickRandomTarget() {
    int col = selectedColumn(m_searchColumn);
    if (col < 0) return;
    DynamicArray<double> values = m_workspace.data().numericValues(col);
    if (values.isEmpty()) return;
    m_searchTarget->setValue(values[QRandomGenerator::global()->bounded(values.size())]);
}

void AlgorithmLab::runSearch() {
    int col = selectedColumn(m_searchColumn);
    if (col < 0) return;
    DynamicArray<double> values = m_workspace.data().numericValues(col);
    double target = m_searchTarget->value();

    DynamicArray<double> sorted = values;
    Sorting::sortAscending(sorted);
    HashTable table(values.size() * 2 + 11);
    for (int i = 0; i < values.size(); ++i)
        table.insert(Dataset::formatNumber(values[i], 6), i);

    SearchResult results[3] = {
        Searching::linearSearch(values, target),
        Searching::binarySearch(sorted, target),
        Searching::hashSearch(table, Dataset::formatNumber(target, 6))};
    const char* worksOn[3] = {"unsorted array", "sorted array", "hash table"};

    m_searchTable->setRowCount(3);
    for (int i = 0; i < 3; ++i) {
        const SearchResult& r = results[i];
        m_searchTable->setItem(i, 0, new QTableWidgetItem(Style::toQ(r.algorithm)));
        m_searchTable->setItem(i, 1, new QTableWidgetItem(worksOn[i]));
        m_searchTable->setItem(i, 2, new QTableWidgetItem(r.found ? "yes" : "no"));
        QString position = !r.found ? "-" : i == 2 ? QString("%1 matches").arg(r.matches)
                                                   : QString("index %1").arg(r.index);
        m_searchTable->setItem(i, 3, new QTableWidgetItem(position));
        m_searchTable->setItem(i, 4, numberItem(QString::number(r.comparisons)));
        m_searchTable->setItem(i, 5, numberItem(QString::number(r.microseconds, 'f', 2)));
    }
    m_searchNote->setText(QString("Searched %1 values. Binary search needed at most log2(n) = %2 comparisons.")
                              .arg(values.size()).arg(std::ceil(std::log2(qMax(2, values.size())))));
}

// ================================ TOP-K ====================================

QWidget* AlgorithmLab::buildTopKTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);

    QHBoxLayout* controls = new QHBoxLayout;
    m_topColumn = new QComboBox;
    m_topColumn->setMinimumWidth(160);
    m_topK = new QSpinBox;
    m_topK->setRange(1, 1000);
    m_topK->setValue(10);
    m_largest = new QRadioButton("Largest");
    QRadioButton* smallest = new QRadioButton("Smallest");
    m_largest->setChecked(true);
    QPushButton* run = new QPushButton("Rank");
    connect(run, &QPushButton::clicked, this, &AlgorithmLab::runTopK);
    controls->addWidget(new QLabel("Column:"));
    controls->addWidget(m_topColumn);
    controls->addWidget(new QLabel("K:"));
    controls->addWidget(m_topK);
    controls->addWidget(m_largest);
    controls->addWidget(smallest);
    controls->addWidget(run);
    controls->addStretch();
    layout->addLayout(controls);

    m_topNote = Style::makeHint("A size-K min-heap keeps the K largest values seen so far: O(n log K) "
                                "instead of sorting everything in O(n log n).");
    layout->addWidget(m_topNote);
    m_topTable = new QTableWidget;
    m_topTable->verticalHeader()->setVisible(false);
    m_topTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_topTable, 1);
    return page;
}

void AlgorithmLab::runTopK() {
    int col = selectedColumn(m_topColumn);
    if (col < 0) return;
    const Dataset& data = m_workspace.data();
    int k = m_topK->value();
    bool largest = m_largest->isChecked();
    DynamicArray<RankedItem> top = largest ? TopK::largest(data, col, k) : TopK::smallest(data, col, k);

    // rank, value, then every column of that row for context
    QStringList headers = {"Rank", Style::toQ(data.columnName(col)), "Row"};
    for (int c = 0; c < data.columnCount(); ++c)
        if (c != col) headers << Style::toQ(data.columnName(c));
    m_topTable->clear();
    m_topTable->setColumnCount(headers.size());
    m_topTable->setHorizontalHeaderLabels(headers);
    m_topTable->setRowCount(top.size());
    for (int i = 0; i < top.size(); ++i) {
        m_topTable->setItem(i, 0, numberItem(QString::number(i + 1)));
        QTableWidgetItem* value = numberItem(Style::formatNumber(top[i].value));
        value->setBackground(Style::AccentLight);
        m_topTable->setItem(i, 1, value);
        m_topTable->setItem(i, 2, numberItem(QString::number(top[i].row + 1)));
        int k2 = 3;
        for (int c = 0; c < data.columnCount(); ++c)
            if (c != col)
                m_topTable->setItem(i, k2++, new QTableWidgetItem(Style::toQ(data.cell(top[i].row, c))));
    }
    m_topTable->resizeColumnsToContents();
    m_workspace.log("Top-" + std::to_string(k) + (largest ? " largest " : " smallest ") + "values of '" +
                    data.columnName(col) + "'");
    emit dataModified();
}

// ================================= BST =====================================

QWidget* AlgorithmLab::buildTreeTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);

    QHBoxLayout* buildRow = new QHBoxLayout;
    m_treeColumn = new QComboBox;
    m_treeColumn->setMinimumWidth(160);
    m_treeSize = new QSpinBox;
    m_treeSize->setRange(3, 63);
    m_treeSize->setValue(15);
    m_treeOrder = new QComboBox;
    m_treeOrder->addItems({"Dataset order", "Random order", "Sorted order (worst case!)"});
    QPushButton* build = new QPushButton("Build tree");
    connect(build, &QPushButton::clicked, this, &AlgorithmLab::buildTree);
    buildRow->addWidget(new QLabel("Column:"));
    buildRow->addWidget(m_treeColumn);
    buildRow->addWidget(new QLabel("Values:"));
    buildRow->addWidget(m_treeSize);
    buildRow->addWidget(new QLabel("Insert order:"));
    buildRow->addWidget(m_treeOrder);
    buildRow->addWidget(build);
    buildRow->addStretch();
    layout->addLayout(buildRow);

    QHBoxLayout* actionRow = new QHBoxLayout;
    m_treeValue = new QLineEdit;
    m_treeValue->setPlaceholderText("value");
    m_treeValue->setMaximumWidth(140);
    QPushButton* search = new QPushButton("Search (animate)");
    QPushButton* insert = new QPushButton("Insert");
    QPushButton* remove = new QPushButton("Delete");
    insert->setProperty("secondary", true);
    remove->setProperty("secondary", true);
    connect(search, &QPushButton::clicked, this, &AlgorithmLab::searchTree);
    connect(m_treeValue, &QLineEdit::returnPressed, this, &AlgorithmLab::searchTree);
    connect(insert, &QPushButton::clicked, this, &AlgorithmLab::insertIntoTree);
    connect(remove, &QPushButton::clicked, this, &AlgorithmLab::deleteFromTree);
    actionRow->addWidget(new QLabel("Value:"));
    actionRow->addWidget(m_treeValue);
    actionRow->addWidget(search);
    actionRow->addWidget(insert);
    actionRow->addWidget(remove);
    actionRow->addStretch();
    layout->addLayout(actionRow);

    m_treeInfo = new QLabel("Build a tree from the first values of a numeric column.");
    m_treeInfo->setWordWrap(true);
    m_treeInfo->setStyleSheet("color:#3b2c85;");
    layout->addWidget(m_treeInfo);

    m_treeWidget = new BSTWidget;
    m_treeWidget->setTree(&m_tree);
    layout->addWidget(m_treeWidget, 1);
    return page;
}

void AlgorithmLab::updateTreeInfo(const QString& message) {
    DynamicArray<double> keys;
    m_tree.inOrder(keys);
    QString inorder;
    for (int i = 0; i < keys.size(); ++i)
        inorder += (i ? ", " : "") + QString::number(keys[i], 'g', 4);
    m_treeInfo->setText(message + QString("<br><b>Nodes:</b> %1 &nbsp; <b>Height:</b> %2 &nbsp; "
                                          "<b>In-order (sorted):</b> %3")
                                      .arg(m_tree.size()).arg(m_tree.height()).arg(inorder));
    m_treeWidget->update();
}

void AlgorithmLab::buildTree() {
    int col = selectedColumn(m_treeColumn);
    if (col < 0) return;
    m_animation->stop();
    m_tree.clear();

    // take the first N *distinct* values so the tree has exactly N nodes
    DynamicArray<double> all = sampleColumn(col, 100000);
    DynamicArray<double> values;
    for (int i = 0; i < all.size() && values.size() < m_treeSize->value(); ++i)
        if (!values.contains(all[i]))
            values.pushBack(all[i]);

    if (m_treeOrder->currentIndex() == 1) {
        for (int i = values.size() - 1; i > 0; --i)
            values.swap(i, QRandomGenerator::global()->bounded(i + 1));
    } else if (m_treeOrder->currentIndex() == 2) {
        Sorting::sortAscending(values);
    }
    for (int i = 0; i < values.size(); ++i)
        m_tree.insert(values[i], i);

    m_treeWidget->setTree(&m_tree);
    if (!values.isEmpty())
        m_treeValue->setText(QString::number(values[values.size() / 2], 'g', 6));
    QString warning = m_treeOrder->currentIndex() == 2
        ? "Sorted input makes every node a right child: the BST degenerates into a linked list (height = n)."
        : "Built by inserting values one by one (smaller -> left, bigger -> right).";
    updateTreeInfo(warning);
}

void AlgorithmLab::searchTree() {
    bool ok = false;
    double key = m_treeValue->text().toDouble(&ok);
    if (!ok || m_tree.isEmpty()) return;

    DynamicArray<double> path;
    m_pathFound = m_tree.search(key, path) != nullptr;
    m_path.clear();
    for (int i = 0; i < path.size(); ++i)
        m_path.append(path[i]);
    m_animationStep = 0;
    m_treeWidget->setPath(m_path, 0, m_pathFound);
    m_animation->start();
}

void AlgorithmLab::animationStep() {
    ++m_animationStep;
    m_treeWidget->setPath(m_path, m_animationStep, m_pathFound);
    if (m_animationStep >= m_path.size()) {
        m_animation->stop();
        QStringList steps;
        for (double k : m_path)
            steps << QString::number(k, 'g', 4);
        updateTreeInfo(QString("Search for <b>%1</b>: %2 after visiting %3 nodes (%4).")
                           .arg(m_treeValue->text(), m_pathFound ? "FOUND" : "not found")
                           .arg(m_path.size()).arg(steps.join(" &rarr; ")));
    }
}

void AlgorithmLab::insertIntoTree() {
    bool ok = false;
    double key = m_treeValue->text().toDouble(&ok);
    if (!ok) return;
    if (m_tree.size() >= 80) {
        updateTreeInfo("The visualizer is limited to 80 nodes.");
        return;
    }
    m_animation->stop();
    m_tree.insert(key, -1);
    m_treeWidget->clearPath();
    updateTreeInfo(QString("Inserted %1.").arg(m_treeValue->text()));
}

void AlgorithmLab::deleteFromTree() {
    bool ok = false;
    double key = m_treeValue->text().toDouble(&ok);
    if (!ok) return;
    m_animation->stop();
    bool removed = m_tree.remove(key);
    m_treeWidget->clearPath();
    updateTreeInfo(removed ? QString("Deleted %1 (a node with two children is replaced by its in-order successor).")
                                 .arg(m_treeValue->text())
                           : QString("%1 is not in the tree.").arg(m_treeValue->text()));
}
