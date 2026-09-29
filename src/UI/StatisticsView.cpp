#include "UI/StatisticsView.h"
#include "UI/Style.h"
#include "Algorithms/Statistics.h"
#include "Algorithms/Correlation.h"
#include "Algorithms/Sorting.h"
#include "Visualization/Histogram.h"
#include "Visualization/BarChart.h"
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QTabWidget>
#include <QStackedWidget>
#include <QGroupBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

using Style::formatNumber;

static QTableWidget* makeTable(const QStringList& headers) {
    QTableWidget* table = new QTableWidget;
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    return table;
}

StatisticsView::StatisticsView(Workspace& workspace, QWidget* parent) : BaseView(workspace, parent) {
    QTabWidget* tabs = new QTabWidget;
    tabs->addTab(buildColumnTab(), "Column statistics");
    tabs->addTab(buildCorrelationTab(), "Correlation matrix");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
}

QWidget* StatisticsView::buildColumnTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);

    QHBoxLayout* controls = new QHBoxLayout;
    m_column = new QComboBox;
    m_column->setMinimumWidth(200);
    m_bins = new QSpinBox;
    m_bins->setRange(2, 60);
    m_bins->setValue(12);
    m_percentile = new QDoubleSpinBox;
    m_percentile->setRange(0, 100);
    m_percentile->setValue(90);
    m_percentile->setSuffix(" th");
    controls->addWidget(new QLabel("Column:"));
    controls->addWidget(m_column);
    controls->addSpacing(20);
    controls->addWidget(new QLabel("Histogram bins:"));
    controls->addWidget(m_bins);
    controls->addSpacing(20);
    controls->addWidget(new QLabel("Custom percentile:"));
    controls->addWidget(m_percentile);
    controls->addStretch();
    layout->addLayout(controls);

    QSplitter* splitter = new QSplitter;
    m_statsTable = makeTable({"Statistic", "Value", "Meaning"});
    m_statsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_statsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_statsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    splitter->addWidget(m_statsTable);

    QWidget* right = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    m_chartStack = new QStackedWidget;
    m_histogram = new Histogram;
    m_topValuesChart = new BarChart;
    m_chartStack->addWidget(m_histogram);
    m_chartStack->addWidget(m_topValuesChart);
    rightLayout->addWidget(m_chartStack, 3);
    m_frequencyTable = makeTable({"Range / Value", "Count", "Percent"});
    rightLayout->addWidget(m_frequencyTable, 2);
    splitter->addWidget(right);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    layout->addWidget(splitter, 1);

    connect(m_column, &QComboBox::currentIndexChanged, this, &StatisticsView::updateColumnStats);
    connect(m_bins, &QSpinBox::valueChanged, this, &StatisticsView::updateColumnStats);
    connect(m_percentile, &QDoubleSpinBox::valueChanged, this, &StatisticsView::updateColumnStats);
    return page;
}

QWidget* StatisticsView::buildCorrelationTab() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->addWidget(Style::makeHint(
        "Pearson r between every pair of numeric columns (only rows where both values are valid). "
        "+1 = perfect positive, 0 = no linear relation, -1 = perfect negative. "
        "Blue = positive, red = negative, darker = stronger."));
    m_matrixTable = new QTableWidget;
    m_matrixTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_matrixTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(m_matrixTable, 3);

    QHBoxLayout* thresholdRow = new QHBoxLayout;
    m_threshold = new QDoubleSpinBox;
    m_threshold->setRange(0.1, 1.0);
    m_threshold->setSingleStep(0.05);
    m_threshold->setValue(Correlation::DEFAULT_THRESHOLD);
    thresholdRow->addWidget(new QLabel("Strong pairs with |r| >="));
    thresholdRow->addWidget(m_threshold);
    thresholdRow->addStretch();
    layout->addLayout(thresholdRow);
    m_pairsTable = makeTable({"Column A", "Column B", "r", "Interpretation"});
    layout->addWidget(m_pairsTable, 2);
    connect(m_threshold, &QDoubleSpinBox::valueChanged, this, &StatisticsView::updateCorrelation);
    return page;
}

void StatisticsView::refresh() {
    bool firstTime = m_column->count() == 0;
    fillColumnCombo(m_column, false);
    if (firstTime) {                       // start on a numeric column, it is more interesting
        DynamicArray<int> numeric = m_workspace.data().numericColumnIndices();
        if (!numeric.isEmpty())
            m_column->setCurrentIndex(m_column->findData(numeric[0]));
    }
    updateColumnStats();
    updateCorrelation();
}

void StatisticsView::addStatRow(const QString& name, const QString& value, const QString& note) {
    int row = m_statsTable->rowCount();
    m_statsTable->insertRow(row);
    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    QFont bold = nameItem->font();
    bold.setBold(true);
    nameItem->setFont(bold);
    m_statsTable->setItem(row, 0, nameItem);
    QTableWidgetItem* valueItem = new QTableWidgetItem(value);
    valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_statsTable->setItem(row, 1, valueItem);
    QTableWidgetItem* noteItem = new QTableWidgetItem(note);
    noteItem->setForeground(QColor("#777"));
    m_statsTable->setItem(row, 2, noteItem);
}

void StatisticsView::updateColumnStats() {
    m_statsTable->setRowCount(0);
    m_frequencyTable->setRowCount(0);
    m_histogram->clearChart();
    m_topValuesChart->clearChart();

    int col = selectedColumn(m_column);
    if (col < 0 || !m_workspace.hasData())
        return;
    const Dataset& data = m_workspace.data();
    QString name = Style::toQ(data.columnName(col));

    int missing = 0;
    for (int r = 0; r < data.rowCount(); ++r)
        if (data.isMissing(r, col)) ++missing;

    if (!data.isNumeric(col)) {
        // ---------- text column: value counts ----------
        DynamicArray<ValueCount> counts = Statistics::valueCounts(data, col, 1000000);
        addStatRow("Type", "Text");
        addStatRow("Filled cells", QString::number(data.rowCount() - missing));
        addStatRow("Missing", QString::number(missing));
        addStatRow("Unique values", QString::number(counts.size()), "counted with the hash table");
        if (!counts.isEmpty())
            addStatRow("Most frequent", Style::toQ(counts[0].value), QString("%1 times").arg(counts[0].count));

        QStringList labels;
        QVector<double> values;
        int filled = data.rowCount() - missing;
        for (int i = 0; i < counts.size(); ++i) {
            if (i < 15) {
                labels << Style::toQ(counts[i].value);
                values << counts[i].count;
            }
            int row = m_frequencyTable->rowCount();
            if (row >= 200) break;
            m_frequencyTable->insertRow(row);
            m_frequencyTable->setItem(row, 0, new QTableWidgetItem(Style::toQ(counts[i].value)));
            m_frequencyTable->setItem(row, 1, new QTableWidgetItem(QString::number(counts[i].count)));
            m_frequencyTable->setItem(row, 2, new QTableWidgetItem(
                QString::number(100.0 * counts[i].count / qMax(1, filled), 'f', 1) + " %"));
        }
        m_topValuesChart->setTitle("Most frequent values of " + name);
        m_topValuesChart->setAxisLabels(name, "count");
        m_topValuesChart->setBars(labels, values);
        m_chartStack->setCurrentWidget(m_topValuesChart);
        return;
    }

    // ---------- numeric column ----------
    DynamicArray<double> values = data.numericValues(col);
    ColumnSummary s = Statistics::summarize(values);
    DynamicArray<double> sorted = values;
    Sorting::sortAscending(sorted);
    double p = m_percentile->value();

    addStatRow("Count", QString::number(s.count), "valid numbers");
    addStatRow("Missing / invalid", QString::number(data.rowCount() - s.count));
    addStatRow("Sum", formatNumber(s.sum));
    addStatRow("Mean", formatNumber(s.mean), "average = sum / n");
    addStatRow("Median", formatNumber(s.median), "middle value (50th percentile)");
    addStatRow("Mode", formatNumber(s.mode), QString("most frequent value (%1 times)").arg(s.modeFrequency));
    addStatRow("Minimum", formatNumber(s.min));
    addStatRow("Maximum", formatNumber(s.max));
    addStatRow("Range", formatNumber(s.range), "max - min");
    addStatRow("Variance", formatNumber(s.variance), "mean of squared distance to the mean");
    addStatRow("Std deviation", formatNumber(s.stdDev), "square root of variance");
    addStatRow("Q1", formatNumber(s.q1), "25th percentile");
    addStatRow("Q3", formatNumber(s.q3), "75th percentile");
    addStatRow("IQR", formatNumber(s.iqr), "Q3 - Q1 (spread of the middle half)");
    addStatRow(QString("P%1").arg(p), formatNumber(Statistics::percentileSorted(sorted, p)), "custom percentile");
    double cv = s.mean != 0 ? 100.0 * s.stdDev / std::fabs(s.mean) : NAN;
    addStatRow("Coeff. of variation", formatNumber(cv, 2) + " %", "std dev / mean");

    DynamicArray<FrequencyBin> bins = Statistics::frequencyDistribution(values, m_bins->value());
    QVector<Histogram::Bin> chartBins;
    for (int i = 0; i < bins.size(); ++i) {
        chartBins.append({bins[i].from, bins[i].to, bins[i].count});
        int row = m_frequencyTable->rowCount();
        m_frequencyTable->insertRow(row);
        m_frequencyTable->setItem(row, 0, new QTableWidgetItem(
            QString("%1  -  %2").arg(formatNumber(bins[i].from, 2), formatNumber(bins[i].to, 2))));
        m_frequencyTable->setItem(row, 1, new QTableWidgetItem(QString::number(bins[i].count)));
        m_frequencyTable->setItem(row, 2, new QTableWidgetItem(
            QString::number(100.0 * bins[i].count / qMax(1, s.count), 'f', 1) + " %"));
    }
    m_histogram->setTitle("Distribution of " + name);
    m_histogram->setAxisLabels(name, "frequency");
    m_histogram->setBins(chartBins);
    m_chartStack->setCurrentWidget(m_histogram);
}

void StatisticsView::updateCorrelation() {
    m_matrixTable->clear();
    m_matrixTable->setRowCount(0);
    m_matrixTable->setColumnCount(0);
    m_pairsTable->setRowCount(0);
    if (!m_workspace.hasData())
        return;

    CorrelationMatrix matrix = Correlation::computeMatrix(m_workspace.data());
    int m = matrix.size();
    QStringList names;
    for (int i = 0; i < m; ++i)
        names << Style::toQ(matrix.names[i]);
    m_matrixTable->setRowCount(m);
    m_matrixTable->setColumnCount(m);
    m_matrixTable->setHorizontalHeaderLabels(names);
    m_matrixTable->setVerticalHeaderLabels(names);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j) {
            double r = matrix.r[i][j];
            QTableWidgetItem* item = new QTableWidgetItem(std::isnan(r) ? "-" : QString::number(r, 'f', 3));
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(Style::correlationColor(r));
            item->setToolTip(names[i] + " vs " + names[j] + ": " + Style::toQ(Correlation::describe(r)));
            m_matrixTable->setItem(i, j, item);
        }

    DynamicArray<StrongPair> pairs = Correlation::strongPairs(matrix, m_threshold->value());
    m_pairsTable->setRowCount(pairs.size());
    for (int i = 0; i < pairs.size(); ++i) {
        m_pairsTable->setItem(i, 0, new QTableWidgetItem(Style::toQ(pairs[i].first)));
        m_pairsTable->setItem(i, 1, new QTableWidgetItem(Style::toQ(pairs[i].second)));
        QTableWidgetItem* rItem = new QTableWidgetItem(QString::number(pairs[i].r, 'f', 3));
        rItem->setBackground(Style::correlationColor(pairs[i].r));
        m_pairsTable->setItem(i, 2, rItem);
        m_pairsTable->setItem(i, 3, new QTableWidgetItem(Style::toQ(Correlation::describe(pairs[i].r))));
    }
}
