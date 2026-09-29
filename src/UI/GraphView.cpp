#include "UI/GraphView.h"
#include "UI/Style.h"
#include "Visualization/GraphWidget.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>

GraphView::GraphView(Workspace& workspace, QWidget* parent) : BaseView(workspace, parent) {
    m_threshold = new QDoubleSpinBox;
    m_threshold->setRange(0.10, 0.99);
    m_threshold->setSingleStep(0.05);
    m_threshold->setValue(Correlation::DEFAULT_THRESHOLD);
    m_startNode = new QComboBox;
    m_startNode->setMinimumWidth(160);
    QPushButton* bfs = new QPushButton("Run BFS");
    QPushButton* dfs = new QPushButton("Run DFS");
    connect(bfs, &QPushButton::clicked, this, &GraphView::runBfs);
    connect(dfs, &QPushButton::clicked, this, &GraphView::runDfs);
    connect(m_threshold, &QDoubleSpinBox::valueChanged, this, &GraphView::rebuildGraph);

    QHBoxLayout* controls = new QHBoxLayout;
    controls->addWidget(new QLabel("Edge when |r| >="));
    controls->addWidget(m_threshold);
    controls->addSpacing(20);
    controls->addWidget(new QLabel("Start column:"));
    controls->addWidget(m_startNode);
    controls->addWidget(bfs);
    controls->addWidget(dfs);
    controls->addStretch();

    m_summary = Style::makeHint("");
    m_widget = new GraphWidget;
    m_output = new QPlainTextEdit;
    m_output->setReadOnly(true);
    m_output->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_output->setStyleSheet("font-family: Consolas, monospace; font-size: 12px;");

    QSplitter* splitter = new QSplitter;
    splitter->addWidget(m_widget);
    splitter->addWidget(m_output);
    splitter->setSizes({700, 420});

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(controls);
    layout->addWidget(m_summary);
    layout->addWidget(splitter, 1);

    m_animation = new QTimer(this);
    m_animation->setInterval(600);
    connect(m_animation, &QTimer::timeout, this, &GraphView::animationStep);
}

void GraphView::refresh() {
    m_matrix = Correlation::computeMatrix(m_workspace.data());
    rebuildGraph();
}

void GraphView::rebuildGraph() {
    m_animation->stop();
    Correlation::buildGraph(m_matrix, m_threshold->value(), m_graph);
    int clusters = m_graph.connectedComponents(m_clusterOf);

    QVector<int> clusterOf;
    for (int i = 0; i < m_clusterOf.size(); ++i)
        clusterOf.append(m_clusterOf[i]);
    m_widget->setGraph(m_graph, clusterOf);

    QString previous = m_startNode->currentText();
    m_startNode->clear();
    for (int i = 0; i < m_graph.nodeCount(); ++i)
        m_startNode->addItem(Style::toQ(m_graph.label(i)), i);
    int index = m_startNode->findText(previous);
    if (index >= 0)
        m_startNode->setCurrentIndex(index);

    m_summary->setText(QString("%1 nodes (numeric columns), %2 edges, %3 clusters found with DFS. "
                               "Blue edge = positive correlation, red = negative; node colour = cluster.")
                           .arg(m_graph.nodeCount()).arg(m_graph.edgeCount()).arg(clusters));
    describeGraph();
}

// Prints the adjacency list and the clusters, so students can see the structure.
void GraphView::describeGraph() {
    QString text = "ADJACENCY LIST\n";
    for (int i = 0; i < m_graph.nodeCount(); ++i) {
        text += "  " + Style::toQ(m_graph.label(i)) + " -> ";
        const LinkedList<Graph::Edge>& edges = m_graph.neighbors(i);
        if (edges.isEmpty())
            text += "(none)";
        for (const LinkedList<Graph::Edge>::Node* n = edges.first(); n != nullptr; n = n->next)
            text += Style::toQ(m_graph.label(n->data.to)) + QString("(%1)  ").arg(n->data.weight, 0, 'f', 2);
        text += "\n";
    }

    int clusters = 0;
    for (int i = 0; i < m_clusterOf.size(); ++i)
        clusters = qMax(clusters, m_clusterOf[i] + 1);
    text += "\nCLUSTERS (connected components)\n";
    for (int c = 0; c < clusters; ++c) {
        QStringList members;
        for (int i = 0; i < m_clusterOf.size(); ++i)
            if (m_clusterOf[i] == c) members << Style::toQ(m_graph.label(i));
        text += QString("  #%1: %2%3\n").arg(c + 1).arg(members.join(", "),
                                                      members.size() == 1 ? "   (isolated)" : "");
    }
    m_output->setPlainText(text);
}

void GraphView::runBfs() {
    int start = m_startNode->currentData().toInt();
    if (m_startNode->currentIndex() < 0) return;
    startTraversal(m_graph.bfs(start), "BFS");
}

void GraphView::runDfs() {
    int start = m_startNode->currentData().toInt();
    if (m_startNode->currentIndex() < 0) return;
    startTraversal(m_graph.dfs(start), "DFS");
}

void GraphView::startTraversal(const DynamicArray<int>& order, const QString& name) {
    m_order.clear();
    for (int i = 0; i < order.size(); ++i)
        m_order.append(order[i]);
    m_traversalName = name;
    m_step = 0;
    m_widget->setVisitOrder(m_order, 0);
    describeGraph();
    m_output->appendPlainText("\n" + name + " from " + m_startNode->currentText() + ":");
    m_animation->start();
    animationStep();
}

void GraphView::animationStep() {
    if (m_step >= m_order.size()) {
        m_animation->stop();
        m_output->appendPlainText(QString("  done - %1 of %2 columns reachable.")
                                      .arg(m_order.size()).arg(m_graph.nodeCount()));
        m_workspace.log(Style::toStd(m_traversalName) + " traversal from '" +
                        Style::toStd(m_startNode->currentText()) + "' reached " +
                        std::to_string(m_order.size()) + " columns");
        emit dataModified();
        return;
    }
    int node = m_order[m_step];
    ++m_step;
    m_widget->setVisitOrder(m_order, m_step);
    m_output->appendPlainText(QString("  %1. visit %2").arg(m_step).arg(Style::toQ(m_graph.label(node))));
}
