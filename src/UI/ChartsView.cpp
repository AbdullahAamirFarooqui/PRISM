#include "UI/ChartsView.h"
#include "UI/Style.h"
#include "Algorithms/Statistics.h"
#include "Algorithms/Correlation.h"
#include "DSA/HashTable.h"
#include "Visualization/LineChart.h"
#include "Visualization/BarChart.h"
#include "Visualization/ScatterPlot.h"
#include "Visualization/Histogram.h"
#include <QComboBox>
#include <QSpinBox>
#include <QStackedWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <algorithm>

ChartsView::ChartsView(Workspace& workspace, QWidget* parent) : BaseView(workspace, parent) {
    m_type = new QComboBox;
    m_type->addItems({"Line chart", "Bar chart", "Scatter plot", "Histogram"});
    m_xColumn = new QComboBox;
    m_yColumn = new QComboBox;
    m_groupColumn = new QComboBox;
    m_bins = new QSpinBox;
    m_bins->setRange(2, 60);
    m_bins->setValue(15);
    for (QComboBox* c : {m_xColumn, m_yColumn, m_groupColumn})
        c->setMinimumWidth(150);

    QHBoxLayout* controls = new QHBoxLayout;
    controls->addWidget(new QLabel("Chart:"));
    controls->addWidget(m_type);
    controls->addSpacing(12);
    controls->addWidget(new QLabel("X:"));
    controls->addWidget(m_xColumn);
    controls->addWidget(new QLabel("Y:"));
    controls->addWidget(m_yColumn);
    controls->addWidget(new QLabel("Group by:"));
    controls->addWidget(m_groupColumn);
    controls->addWidget(new QLabel("Bins:"));
    controls->addWidget(m_bins);
    controls->addStretch();

    m_line = new LineChart;
    m_bar = new BarChart;
    m_scatter = new ScatterPlot;
    m_histogram = new Histogram;
    m_stack = new QStackedWidget;
    m_stack->addWidget(m_line);
    m_stack->addWidget(m_bar);
    m_stack->addWidget(m_scatter);
    m_stack->addWidget(m_histogram);

    m_info = Style::makeHint("");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(controls);
    layout->addWidget(m_info);
    layout->addWidget(m_stack, 1);

    connect(m_type, &QComboBox::currentIndexChanged, this, &ChartsView::updateChart);
    connect(m_xColumn, &QComboBox::currentIndexChanged, this, &ChartsView::updateChart);
    connect(m_yColumn, &QComboBox::currentIndexChanged, this, &ChartsView::updateChart);
    connect(m_groupColumn, &QComboBox::currentIndexChanged, this, &ChartsView::updateChart);
    connect(m_bins, &QSpinBox::valueChanged, this, &ChartsView::updateChart);
}

void ChartsView::refresh() {
    fillColumnCombo(m_xColumn, true, "(row number)");
    fillColumnCombo(m_yColumn, true);

    // group-by: only text columns make sense
    QString previous = m_groupColumn->currentText();
    m_groupColumn->blockSignals(true);
    m_groupColumn->clear();
    m_groupColumn->addItem("(none)", -1);
    const Dataset& data = m_workspace.data();
    for (int c = 0; c < data.columnCount(); ++c)
        if (!data.isNumeric(c))
            m_groupColumn->addItem(Style::toQ(data.columnName(c)), c);
    int index = m_groupColumn->findText(previous);
    m_groupColumn->setCurrentIndex(index >= 0 ? index : 0);
    m_groupColumn->blockSignals(false);

    // A nicer default Y than the first column: the second numeric column if there is one.
    if (m_yColumn->count() > 1 && m_yColumn->currentIndex() == 0 && previous.isEmpty())
        m_yColumn->setCurrentIndex(1);
    updateChart();
}

void ChartsView::updateChart() {
    int type = m_type->currentIndex();
    m_stack->setCurrentIndex(type);
    m_xColumn->setEnabled(type == 0 || type == 2);
    m_groupColumn->setEnabled(type == 0 || type == 1);
    m_bins->setEnabled(type == 3);

    int xCol = selectedColumn(m_xColumn);
    int yCol = selectedColumn(m_yColumn);
    int groupCol = selectedColumn(m_groupColumn);
    m_info->clear();
    if (!m_workspace.hasData() || yCol < 0) {
        m_line->clearChart();
        m_bar->clearChart();
        m_scatter->clearChart();
        m_histogram->clearChart();
        return;
    }
    switch (type) {
    case 0: drawLine(xCol, yCol, groupCol); break;
    case 1: drawBar(yCol, groupCol); break;
    case 2: drawScatter(xCol, yCol); break;
    default: drawHistogram(yCol); break;
    }
}

void ChartsView::drawLine(int xCol, int yCol, int groupCol) {
    const Dataset& data = m_workspace.data();
    QString xName = xCol < 0 ? "row number" : Style::toQ(data.columnName(xCol));
    QString yName = Style::toQ(data.columnName(yCol));

    // Collect (x, y) for every row, split into groups.
    // groupOf(row) -> the key used in a HashTable to collect rows per group.
    HashTable groups;
    for (int r = 0; r < data.rowCount(); ++r)
        groups.insert(groupCol < 0 ? std::string("all") : data.cell(r, groupCol), r);
    DynamicArray<std::string> keys;
    groups.keys(keys);
    QStringList names;
    for (int i = 0; i < keys.size(); ++i)
        names << Style::toQ(keys[i]);
    names.sort();

    const int maxSeries = 10;
    QVector<LineChart::Series> seriesList;
    for (int g = 0; g < names.size() && g < maxSeries; ++g) {
        const DynamicArray<int>* rows = groups.find(Style::toStd(names[g]));
        LineChart::Series series;
        series.name = names[g].isEmpty() ? "(missing)" : names[g];
        for (int i = 0; i < rows->size(); ++i) {
            int r = (*rows)[i];
            double x = r + 1, y;
            if (xCol >= 0 && !data.numberAt(r, xCol, x)) continue;
            if (!data.numberAt(r, yCol, y)) continue;
            series.points.append(QPointF(x, y));
        }
        std::sort(series.points.begin(), series.points.end(),
                  [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); });
        seriesList.append(series);
    }
    if (names.size() > maxSeries)
        m_info->setText(QString("Showing the first %1 of %2 groups.").arg(maxSeries).arg(names.size()));

    m_line->setTitle(yName + " over " + xName);
    m_line->setAxisLabels(xName, yName);
    m_line->setSeries(seriesList);
}

void ChartsView::drawBar(int yCol, int groupCol) {
    const Dataset& data = m_workspace.data();
    QString yName = Style::toQ(data.columnName(yCol));
    QStringList labels;
    QVector<double> values;

    if (groupCol < 0) {
        // No grouping: simply the first 30 rows
        for (int r = 0; r < data.rowCount() && labels.size() < 30; ++r) {
            double y;
            if (data.numberAt(r, yCol, y)) {
                labels << QString("row %1").arg(r + 1);
                values << y;
            }
        }
        m_bar->setTitle(yName + " (first 30 rows)");
        m_info->setText("Tip: choose a 'Group by' text column to see the average per group.");
    } else {
        // Group rows by the text column with the hash table, then average each group
        HashTable groups;
        for (int r = 0; r < data.rowCount(); ++r)
            if (!data.isMissing(r, groupCol))
                groups.insert(data.cell(r, groupCol), r);
        DynamicArray<std::string> keys;
        groups.keys(keys);

        QVector<QPair<double, QString>> averages;
        for (int k = 0; k < keys.size(); ++k) {
            const DynamicArray<int>* rows = groups.find(keys[k]);
            DynamicArray<double> ys;
            double y;
            for (int i = 0; i < rows->size(); ++i)
                if (data.numberAt((*rows)[i], yCol, y))
                    ys.pushBack(y);
            if (!ys.isEmpty())
                averages.append({Statistics::mean(ys), Style::toQ(keys[k])});
        }
        std::sort(averages.begin(), averages.end(),
                  [](const QPair<double, QString>& a, const QPair<double, QString>& b) { return a.first > b.first; });
        for (int i = 0; i < averages.size() && i < 25; ++i) {
            labels << averages[i].second;
            values << averages[i].first;
        }
        QString groupName = Style::toQ(data.columnName(groupCol));
        m_bar->setTitle("Average " + yName + " by " + groupName);
        m_info->setText(QString("%1 groups found with the hash table.").arg(keys.size()));
    }
    m_bar->setAxisLabels(groupCol < 0 ? "row" : Style::toQ(data.columnName(groupCol)), yName);
    m_bar->setBars(labels, values);
}

void ChartsView::drawScatter(int xCol, int yCol) {
    const Dataset& data = m_workspace.data();
    QString xName = xCol < 0 ? "row number" : Style::toQ(data.columnName(xCol));
    QString yName = Style::toQ(data.columnName(yCol));

    QVector<QPointF> points;
    DynamicArray<double> xs, ys;
    for (int r = 0; r < data.rowCount(); ++r) {
        double x = r + 1, y;
        if (xCol >= 0 && !data.numberAt(r, xCol, x)) continue;
        if (!data.numberAt(r, yCol, y)) continue;
        points.append(QPointF(x, y));
        xs.pushBack(x);
        ys.pushBack(y);
    }
    m_scatter->setTitle(yName + " vs " + xName);
    m_scatter->setAxisLabels(xName, yName);
    m_scatter->setPoints(points);

    double slope, intercept;
    double r = Correlation::pearson(xs, ys);
    if (Correlation::linearFit(xs, ys, slope, intercept)) {
        m_scatter->setTrendLine(slope, intercept, QString("r = %1   y = %2x + %3")
                                    .arg(r, 0, 'f', 3).arg(slope, 0, 'g', 4).arg(intercept, 0, 'g', 4));
        m_info->setText(QString("%1 points. Pearson r = %2 (%3).").arg(points.size()).arg(r, 0, 'f', 3)
                            .arg(Style::toQ(Correlation::describe(r))));
    } else {
        m_scatter->clearTrendLine();
    }
}

void ChartsView::drawHistogram(int yCol) {
    const Dataset& data = m_workspace.data();
    QString yName = Style::toQ(data.columnName(yCol));
    DynamicArray<FrequencyBin> bins = Statistics::frequencyDistribution(data.numericValues(yCol), m_bins->value());
    QVector<Histogram::Bin> chartBins;
    for (int i = 0; i < bins.size(); ++i)
        chartBins.append({bins[i].from, bins[i].to, bins[i].count});
    m_histogram->setTitle("Histogram of " + yName);
    m_histogram->setAxisLabels(yName, "frequency");
    m_histogram->setBins(chartBins);
}
