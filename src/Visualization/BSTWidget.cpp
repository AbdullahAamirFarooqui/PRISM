#include "Visualization/BSTWidget.h"
#include <QPainter>
#include <QHash>

BSTWidget::BSTWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(500, 320);
}

void BSTWidget::setTree(const BST* tree) {
    m_tree = tree;
    clearPath();
}

void BSTWidget::setPath(const QVector<double>& path, int visibleSteps, bool found) {
    m_path = path;
    m_visibleSteps = visibleSteps;
    m_found = found;
    update();
}

void BSTWidget::clearPath() {
    m_path.clear();
    m_visibleSteps = 0;
    update();
}

// Recursive in-order walk that records (column, row) grid coordinates.
void BSTWidget::layout(const BST::Node* node, int depth, int& inOrderIndex,
                       QVector<const BST::Node*>& nodes, QVector<QPointF>& grid) const {
    if (node == nullptr)
        return;
    layout(node->left, depth + 1, inOrderIndex, nodes, grid);
    nodes.append(node);
    grid.append(QPointF(inOrderIndex++, depth));
    layout(node->right, depth + 1, inOrderIndex, nodes, grid);
}

void BSTWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    if (m_tree == nullptr || m_tree->isEmpty()) {
        painter.setPen(QColor("#999"));
        painter.drawText(rect(), Qt::AlignCenter, "Build a tree to see it here.");
        return;
    }

    QVector<const BST::Node*> nodes;
    QVector<QPointF> grid;
    int index = 0;
    layout(m_tree->root(), 0, index, nodes, grid);

    int levels = m_tree->height();
    double xStep = (width() - 40.0) / qMax(1, nodes.size());
    double yStep = (height() - 60.0) / qMax(1, levels);
    double radius = qBound(9.0, xStep * 0.45, 20.0);

    // screen position of every node (lookup by pointer)
    QHash<const BST::Node*, QPointF> screen;
    for (int i = 0; i < nodes.size(); ++i)
        screen[nodes[i]] = QPointF(20 + xStep * (grid[i].x() + 0.5), 30 + yStep * grid[i].y());

    // which keys are on the visible part of the search path?
    QHash<double, int> pathStep;
    for (int s = 0; s < m_visibleSteps && s < m_path.size(); ++s)
        pathStep[m_path[s]] = s;
    bool finished = m_visibleSteps >= m_path.size() && !m_path.isEmpty();

    // edges first (so circles are drawn on top)
    for (const BST::Node* node : nodes) {
        const BST::Node* children[2] = {node->left, node->right};
        for (const BST::Node* child : children) {
            if (child == nullptr)
                continue;
            bool onPath = pathStep.contains(node->key) && pathStep.contains(child->key);
            painter.setPen(QPen(onPath ? QColor("#e67700") : QColor("#b8b5cc"), onPath ? 3 : 1.5));
            painter.drawLine(screen[node], screen[child]);
        }
    }

    QFont small = font();
    small.setPointSizeF(qMax(6.5, radius / 2.4));
    painter.setFont(small);
    for (const BST::Node* node : nodes) {
        QPointF p = screen[node];
        QColor fill("#ecebfb");
        QColor border("#5b4fcf");
        if (pathStep.contains(node->key)) {
            fill = QColor("#ffd43b");
            border = QColor("#e67700");
            bool last = pathStep[node->key] == m_path.size() - 1;
            if (finished && last)
                fill = m_found ? QColor("#8ce99a") : QColor("#ffa8a8");
        }
        painter.setPen(QPen(border, 2));
        painter.setBrush(fill);
        painter.drawEllipse(p, radius, radius);
        painter.setPen(QColor("#222"));
        painter.drawText(QRectF(p.x() - radius - 6, p.y() - radius, 2 * radius + 12, 2 * radius),
                         Qt::AlignCenter, QString::number(node->key, 'g', 4));
    }
}
