#include "Visualization/GraphWidget.h"
#include "Visualization/ChartWidget.h"
#include "DSA/Graph.h"
#include <QPainter>
#include <QtMath>

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(420, 360);
}

void GraphWidget::setGraph(const Graph& graph, const QVector<int>& clusterOf) {
    m_labels.clear();
    m_edges.clear();
    for (int i = 0; i < graph.nodeCount(); ++i) {
        m_labels << QString::fromStdString(graph.label(i));
        for (const LinkedList<Graph::Edge>::Node* n = graph.neighbors(i).first(); n != nullptr; n = n->next)
            if (n->data.to > i)                        // each undirected edge once
                m_edges.append({i, n->data.to, n->data.weight});
    }
    m_clusterOf = clusterOf;
    clearVisit();
}

void GraphWidget::setVisitOrder(const QVector<int>& order, int visibleSteps) {
    m_visitOrder = order;
    m_visibleSteps = visibleSteps;
    update();
}

void GraphWidget::clearVisit() {
    m_visitOrder.clear();
    m_visibleSteps = 0;
    update();
}

void GraphWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    int n = m_labels.size();
    if (n == 0) {
        painter.setPen(QColor("#999"));
        painter.drawText(rect(), Qt::AlignCenter, "No numeric columns to build a graph from.");
        return;
    }

    // Place the nodes evenly on a circle.
    QPointF center(width() / 2.0, height() / 2.0);
    double radius = qMin(width(), height()) / 2.0 - 70;
    QVector<QPointF> pos(n);
    for (int i = 0; i < n; ++i) {
        double angle = 2 * M_PI * i / n - M_PI / 2;
        pos[i] = center + QPointF(radius * qCos(angle), radius * qSin(angle));
    }

    // Edges
    for (const EdgeInfo& e : m_edges) {
        QColor color = e.r >= 0 ? QColor("#2f6fdf") : QColor("#d9480f");
        painter.setPen(QPen(color, 1 + 5 * qAbs(e.r)));
        painter.drawLine(pos[e.a], pos[e.b]);

        QPointF mid = (pos[e.a] + pos[e.b]) / 2;
        QString text = QString::number(e.r, 'f', 2);
        QRectF box(mid.x() - 20, mid.y() - 9, 40, 18);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 220));
        painter.drawRoundedRect(box, 4, 4);
        painter.setPen(color.darker(120));
        painter.drawText(box, Qt::AlignCenter, text);
    }

    // Which nodes were already reached by the BFS/DFS animation?
    QVector<int> visitStep(n, -1);
    for (int s = 0; s < m_visibleSteps && s < m_visitOrder.size(); ++s)
        visitStep[m_visitOrder[s]] = s;

    // Nodes
    double nodeRadius = 26;
    for (int i = 0; i < n; ++i) {
        int cluster = i < m_clusterOf.size() ? m_clusterOf[i] : 0;
        QColor fill = ChartWidget::seriesColor(cluster).lighter(165);
        QColor border = ChartWidget::seriesColor(cluster);
        bool visited = visitStep[i] >= 0;
        if (visited) {
            fill = QColor("#ffd43b");
            border = QColor("#e67700");
        }
        painter.setPen(QPen(border, visited ? 4 : 2));
        painter.setBrush(fill);
        painter.drawEllipse(pos[i], nodeRadius, nodeRadius);

        if (visited) {                      // visiting order number
            painter.setPen(QColor("#7a4100"));
            QFont f = font();
            f.setBold(true);
            painter.setFont(f);
            painter.drawText(QRectF(pos[i].x() - nodeRadius, pos[i].y() - 10, 2 * nodeRadius, 20),
                             Qt::AlignCenter, QString::number(visitStep[i] + 1));
            painter.setFont(font());
        }

        // label outside the circle
        QPointF dir = pos[i] - center;
        double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
        // push the label further out for nodes on the left/right side (wide text)
        double sideways = len > 0 ? qAbs(dir.x()) / len : 0;
        QPointF labelPos = len > 0 ? pos[i] + dir / len * (nodeRadius + 14 + 48 * sideways)
                                   : pos[i] + QPointF(0, nodeRadius + 16);
        QString label = m_labels[i];
        QRectF textBox(labelPos.x() - 70, labelPos.y() - 10, 140, 20);
        painter.setPen(QColor("#222"));
        painter.drawText(textBox, Qt::AlignCenter, label);
    }
}
