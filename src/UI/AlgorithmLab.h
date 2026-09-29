#ifndef PRISM_ALGORITHM_LAB_H
#define PRISM_ALGORITHM_LAB_H

#include "UI/BaseView.h"
#include "DSA/BST.h"
#include "Algorithms/Sorting.h"
#include <QVector>

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QTableWidget;
class QLabel;
class QLineEdit;
class QTimer;
class QRadioButton;
class BarChart;
class BSTWidget;

// ---------------------------------------------------------------------------
// AlgorithmLab - hands-on comparison of the algorithms on REAL column data.
//   Sorting   : all six sorters on the same input, comparisons/swaps/time
//   Searching : linear vs binary vs hash search for one value
//   Top-K     : heap based ranking of the largest / smallest values
//   BST       : build a small tree from the column and animate searches
// ---------------------------------------------------------------------------
class AlgorithmLab : public BaseView {
    Q_OBJECT
public:
    explicit AlgorithmLab(Workspace& workspace, QWidget* parent = nullptr);
    ~AlgorithmLab() override;
    QString viewName() const override { return "Algorithm Lab"; }

protected:
    void refresh() override;

private slots:
    void runSortBenchmark();
    void updateSortChart();
    void runSearch();
    void pickRandomTarget();
    void runTopK();
    void buildTree();
    void searchTree();
    void insertIntoTree();
    void deleteFromTree();
    void animationStep();

private:
    QWidget* buildSortTab();
    QWidget* buildSearchTab();
    QWidget* buildTopKTab();
    QWidget* buildTreeTab();
    DynamicArray<double> sampleColumn(int col, int maxCount) const;
    void updateTreeInfo(const QString& message);

    DynamicArray<Sorter*> m_sorters;       // polymorphic list of algorithms
    QVector<SortStats> m_lastResults;

    // sorting tab
    QComboBox* m_sortColumn;
    QSpinBox* m_sortSize;
    QComboBox* m_sortOrder;
    QComboBox* m_sortMetric;
    QTableWidget* m_sortTable;
    BarChart* m_sortChart;
    QLabel* m_sortNote;

    // searching tab
    QComboBox* m_searchColumn;
    QDoubleSpinBox* m_searchTarget;
    QTableWidget* m_searchTable;
    QLabel* m_searchNote;

    // top-k tab
    QComboBox* m_topColumn;
    QSpinBox* m_topK;
    QRadioButton* m_largest;
    QTableWidget* m_topTable;
    QLabel* m_topNote;

    // BST tab
    QComboBox* m_treeColumn;
    QSpinBox* m_treeSize;
    QComboBox* m_treeOrder;
    QLineEdit* m_treeValue;
    QLabel* m_treeInfo;
    BSTWidget* m_treeWidget;
    BST m_tree;
    QTimer* m_animation;
    QVector<double> m_path;
    int m_animationStep = 0;
    bool m_pathFound = false;
};

#endif // PRISM_ALGORITHM_LAB_H
