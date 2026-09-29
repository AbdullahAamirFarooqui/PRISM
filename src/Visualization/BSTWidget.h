#ifndef PRISM_BST_WIDGET_H
#define PRISM_BST_WIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include "DSA/BST.h"

// ---------------------------------------------------------------------------
// BSTWidget - draws a (small) binary search tree and animates a search path.
//
// Layout trick: a node's x position = its position in the IN-ORDER traversal
// (so the picture is sorted left to right), y position = its depth.
// ---------------------------------------------------------------------------
class BSTWidget : public QWidget {
    Q_OBJECT
public:
    explicit BSTWidget(QWidget* parent = nullptr);

    void setTree(const BST* tree);        // the widget does NOT own the tree
    void setPath(const QVector<double>& path, int visibleSteps, bool found);
    void clearPath();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const BST* m_tree = nullptr;
    QVector<double> m_path;
    int m_visibleSteps = 0;
    bool m_found = false;

    void layout(const BST::Node* node, int depth, int& inOrderIndex,
                QVector<const BST::Node*>& nodes, QVector<QPointF>& grid) const;
};

#endif // PRISM_BST_WIDGET_H
