#ifndef PRISM_COMPARE_VIEW_H
#define PRISM_COMPARE_VIEW_H

#include "UI/BaseView.h"

class QComboBox;
class QTableWidget;
class QLabel;
class QPushButton;
class LineChart;
class Histogram;

// ---------------------------------------------------------------------------
// CompareView - side-by-side comparison of the active dataset (A) with a
// second dataset (B), e.g. weather_2024 vs weather_2025.
// Shows the statistics of a common column for A and B and their difference,
// plus both series drawn on the same line chart.
// ---------------------------------------------------------------------------
class CompareView : public BaseView {
    Q_OBJECT
public:
    explicit CompareView(Workspace& workspace, QWidget* parent = nullptr);
    QString viewName() const override { return "Compare"; }

signals:
    void loadComparisonRequested();

protected:
    void refresh() override;

private slots:
    void updateComparison();

private:
    QComboBox* m_column;
    QLabel* m_info;
    QPushButton* m_loadButton;
    QTableWidget* m_table;
    LineChart* m_chart;
};

#endif // PRISM_COMPARE_VIEW_H
