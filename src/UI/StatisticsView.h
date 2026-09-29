#ifndef PRISM_STATISTICS_VIEW_H
#define PRISM_STATISTICS_VIEW_H

#include "UI/BaseView.h"

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QTableWidget;
class QLabel;
class QStackedWidget;
class Histogram;
class BarChart;

// ---------------------------------------------------------------------------
// StatisticsView - two sub tabs:
//   "Column statistics" : mean, median, mode, quartiles ... + histogram and
//                         frequency table (text columns: most frequent values)
//   "Correlation"       : Pearson correlation matrix as a coloured heat map
//                         plus the strongest pairs
// ---------------------------------------------------------------------------
class StatisticsView : public BaseView {
    Q_OBJECT
public:
    explicit StatisticsView(Workspace& workspace, QWidget* parent = nullptr);
    QString viewName() const override { return "Statistics"; }

protected:
    void refresh() override;

private slots:
    void updateColumnStats();
    void updateCorrelation();

private:
    QWidget* buildColumnTab();
    QWidget* buildCorrelationTab();
    void addStatRow(const QString& name, const QString& value, const QString& note = QString());

    // column tab
    QComboBox* m_column;
    QSpinBox* m_bins;
    QDoubleSpinBox* m_percentile;
    QTableWidget* m_statsTable;
    QTableWidget* m_frequencyTable;
    QStackedWidget* m_chartStack;
    Histogram* m_histogram;
    BarChart* m_topValuesChart;

    // correlation tab
    QTableWidget* m_matrixTable;
    QTableWidget* m_pairsTable;
    QDoubleSpinBox* m_threshold;
};

#endif // PRISM_STATISTICS_VIEW_H
