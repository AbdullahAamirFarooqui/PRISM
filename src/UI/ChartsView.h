#ifndef PRISM_CHARTS_VIEW_H
#define PRISM_CHARTS_VIEW_H

#include "UI/BaseView.h"

class QComboBox;
class QSpinBox;
class QStackedWidget;
class QLabel;
class LineChart;
class BarChart;
class ScatterPlot;
class Histogram;

// ---------------------------------------------------------------------------
// ChartsView - interactive charts.
//   Line      : Y against X (or row number), optionally one line per group
//   Bar       : average of Y for every value of a text column (grouping with
//               the HashTable)
//   Scatter   : X vs Y with a linear trend line and Pearson r
//   Histogram : distribution of Y
// ---------------------------------------------------------------------------
class ChartsView : public BaseView {
    Q_OBJECT
public:
    explicit ChartsView(Workspace& workspace, QWidget* parent = nullptr);
    QString viewName() const override { return "Charts"; }

protected:
    void refresh() override;

private slots:
    void updateChart();

private:
    void drawLine(int xCol, int yCol, int groupCol);
    void drawBar(int yCol, int groupCol);
    void drawScatter(int xCol, int yCol);
    void drawHistogram(int yCol);

    QComboBox* m_type;
    QComboBox* m_xColumn;
    QComboBox* m_yColumn;
    QComboBox* m_groupColumn;
    QSpinBox* m_bins;
    QLabel* m_info;
    QStackedWidget* m_stack;
    LineChart* m_line;
    BarChart* m_bar;
    ScatterPlot* m_scatter;
    Histogram* m_histogram;
};

#endif // PRISM_CHARTS_VIEW_H
