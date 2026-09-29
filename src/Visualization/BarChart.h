#ifndef PRISM_BAR_CHART_H
#define PRISM_BAR_CHART_H

#include "Visualization/ChartWidget.h"
#include <QStringList>
#include <QVector>

// Vertical bar chart: one bar per category label.
class BarChart : public ChartWidget {
    Q_OBJECT
public:
    explicit BarChart(QWidget* parent = nullptr);

    void setBars(const QStringList& labels, const QVector<double>& values);
    void clearChart();
    void setShowValues(bool show) { m_showValues = show; update(); }

protected:
    bool hasData() const override { return !m_values.isEmpty(); }
    void dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const override;
    void drawContent(QPainter& painter, const QRectF& plot) override;
    bool numericXAxis() const override { return false; }
    void drawCustomXAxis(QPainter& painter, const QRectF& plot) override;
    int bottomMargin() const override { return 80; }

private:
    QStringList m_labels;
    QVector<double> m_values;
    bool m_showValues = true;
};

#endif // PRISM_BAR_CHART_H
