#include "UI/CompareView.h"
#include "UI/Style.h"
#include "Algorithms/Statistics.h"
#include "Visualization/LineChart.h"
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <cmath>

CompareView::CompareView(Workspace& workspace, QWidget* parent) : BaseView(workspace, parent) {
    m_column = new QComboBox;
    m_column->setMinimumWidth(200);
    m_loadButton = new QPushButton("Load comparison dataset...");
    connect(m_loadButton, &QPushButton::clicked, this, &CompareView::loadComparisonRequested);
    m_info = Style::makeHint("");

    QHBoxLayout* controls = new QHBoxLayout;
    controls->addWidget(m_loadButton);
    controls->addSpacing(16);
    controls->addWidget(new QLabel("Common column:"));
    controls->addWidget(m_column);
    controls->addStretch();

    m_table = new QTableWidget;
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Statistic", "Dataset A", "Dataset B", "B - A"});
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_chart = new LineChart;

    QSplitter* splitter = new QSplitter;
    splitter->addWidget(m_table);
    splitter->addWidget(m_chart);
    splitter->setSizes({420, 700});

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(controls);
    layout->addWidget(m_info);
    layout->addWidget(splitter, 1);

    connect(m_column, &QComboBox::currentIndexChanged, this, &CompareView::updateComparison);
}

void CompareView::refresh() {
    QString previous = m_column->currentText();
    m_column->blockSignals(true);
    m_column->clear();

    const Dataset& a = m_workspace.data();
    const Dataset& b = m_workspace.comparison();
    if (m_workspace.hasData() && m_workspace.hasComparison()) {
        // columns that are numeric in both datasets (matched by name)
        for (int c = 0; c < a.columnCount(); ++c) {
            int other = b.findColumn(a.columnName(c));
            if (a.isNumeric(c) && other >= 0 && b.isNumeric(other))
                m_column->addItem(Style::toQ(a.columnName(c)), c);
        }
        m_info->setText(QString("A = <b>%1</b> (%2 rows)     B = <b>%3</b> (%4 rows)")
                            .arg(Style::toQ(a.name())).arg(a.rowCount())
                            .arg(Style::toQ(b.name())).arg(b.rowCount()));
    } else {
        m_info->setText("Load a second CSV file with the same column names (for example weather_2024.csv "
                        "while weather_2025.csv is open) to compare them.");
    }
    int index = m_column->findText(previous);
    m_column->setCurrentIndex(index >= 0 ? index : 0);
    m_column->blockSignals(false);
    updateComparison();
}

void CompareView::updateComparison() {
    m_table->setRowCount(0);
    m_chart->clearChart();
    int colA = selectedColumn(m_column);
    if (colA < 0 || !m_workspace.hasComparison())
        return;

    const Dataset& a = m_workspace.data();
    const Dataset& b = m_workspace.comparison();
    std::string name = a.columnName(colA);
    int colB = b.findColumn(name);

    ColumnSummary sa = Statistics::summarize(a.numericValues(colA));
    ColumnSummary sb = Statistics::summarize(b.numericValues(colB));

    struct Row { const char* label; double va; double vb; };
    Row rows[] = {{"Count", double(sa.count), double(sb.count)}, {"Mean", sa.mean, sb.mean},
                  {"Median", sa.median, sb.median}, {"Std deviation", sa.stdDev, sb.stdDev},
                  {"Minimum", sa.min, sb.min}, {"Maximum", sa.max, sb.max},
                  {"Range", sa.range, sb.range}, {"Q1", sa.q1, sb.q1}, {"Q3", sa.q3, sb.q3}};
    for (const Row& r : rows) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(r.label));
        m_table->setItem(row, 1, new QTableWidgetItem(Style::formatNumber(r.va, 3)));
        m_table->setItem(row, 2, new QTableWidgetItem(Style::formatNumber(r.vb, 3)));
        double diff = r.vb - r.va;
        QTableWidgetItem* diffItem = new QTableWidgetItem((diff > 0 ? "+" : "") + Style::formatNumber(diff, 3));
        if (!std::isnan(diff) && diff != 0)
            diffItem->setForeground(diff > 0 ? Style::Positive : Style::Negative);
        m_table->setItem(row, 3, diffItem);
    }

    // Both datasets on one chart, x = row number
    QVector<LineChart::Series> series(2);
    series[0].name = "A: " + Style::toQ(a.name());
    series[1].name = "B: " + Style::toQ(b.name());
    double v;
    for (int r = 0; r < a.rowCount(); ++r)
        if (a.numberAt(r, colA, v)) series[0].points.append(QPointF(r + 1, v));
    for (int r = 0; r < b.rowCount(); ++r)
        if (b.numberAt(r, colB, v)) series[1].points.append(QPointF(r + 1, v));
    m_chart->setTitle(Style::toQ(name) + ": A vs B");
    m_chart->setAxisLabels("row number", Style::toQ(name));
    m_chart->setSeries(series);
}
