#ifndef PRISM_GRAPH_WIDGET_H
#define PRISM_GRAPH_WIDGET_H

#include <QWidget>
#include <QStringList>
#include <QVector>

class Graph;

// ---------------------------------------------------------------------------
// GraphWidget - draws the correlation network.
//   nodes  : placed on a circle, coloured by cluster (DFS components)
//   edges  : blue = positive r, red = negative r, thicker = stronger
//   visit  : BFS/DFS order can be highlighted step by step (animation)
// ---------------------------------------------------------------------------
class GraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit GraphWidget(QWidget* parent = nullptr);

    // Copies what it needs from the Graph (so the widget never holds a dangling pointer).
    void setGraph(const Graph& graph, const QVector<int>& clusterOf);
    void setVisitOrder(const QVector<int>& order, int visibleSteps);
    void clearVisit();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct EdgeInfo {
        int a;
        int b;
        double r;
    };

    QStringList m_labels;
    QVector<EdgeInfo> m_edges;
    QVector<int> m_clusterOf;
    QVector<int> m_visitOrder;
    int m_visibleSteps = 0;
};

#endif // PRISM_GRAPH_WIDGET_H
