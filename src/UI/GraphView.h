#ifndef PRISM_GRAPH_VIEW_H
#define PRISM_GRAPH_VIEW_H

#include "UI/BaseView.h"
#include "DSA/Graph.h"
#include "Algorithms/Correlation.h"
#include <QVector>

class QComboBox;
class QDoubleSpinBox;
class QPlainTextEdit;
class QLabel;
class QTimer;
class GraphWidget;

// ---------------------------------------------------------------------------
// GraphView - the dataset relationship network.
//   node = numeric column, edge = |r| >= threshold
//   BFS  : everything reachable from a column, level by level (uses Queue)
//   DFS  : goes deep first (recursive); also used to find clusters
// ---------------------------------------------------------------------------
class GraphView : public BaseView {
    Q_OBJECT
public:
    explicit GraphView(Workspace& workspace, QWidget* parent = nullptr);
    QString viewName() const override { return "Relationship Graph"; }

protected:
    void refresh() override;

private slots:
    void rebuildGraph();
    void runBfs();
    void runDfs();
    void animationStep();

private:
    void startTraversal(const DynamicArray<int>& order, const QString& name);
    void describeGraph();

    CorrelationMatrix m_matrix;
    Graph m_graph;
    DynamicArray<int> m_clusterOf;

    QDoubleSpinBox* m_threshold;
    QComboBox* m_startNode;
    GraphWidget* m_widget;
    QPlainTextEdit* m_output;
    QLabel* m_summary;
    QTimer* m_animation;
    QVector<int> m_order;
    int m_step = 0;
    QString m_traversalName;
};

#endif // PRISM_GRAPH_VIEW_H
