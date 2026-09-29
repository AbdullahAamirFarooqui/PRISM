#ifndef PRISM_SCATTER_PLOT_H
#define PRISM_SCATTER_PLOT_H

#include "Visualization/ChartWidget.h"
#include <QVector>
#include <QPointF>

// Scatter plot (x, y points) with an optional linear trend line.
class ScatterPlot : public ChartWidget {
    Q_OBJECT
public:
    explicit ScatterPlot(QWidget* parent = nullptr);

    void setPoints(const QVector<QPointF>& points);
    void setTrendLine(double slope, double intercept, const QString& info);
    void clearTrendLine();
    void clearChart();

protected:
    bool hasData() const override { return !m_points.isEmpty(); }
    void dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const override;
    void drawContent(QPainter& painter, const QRectF& plot) override;

private:
    QVector<QPointF> m_points;
    bool m_hasTrend = false;
    double m_slope = 0;
    double m_intercept = 0;
    QString m_trendInfo;
};

#endif // PRISM_SCATTER_PLOT_H
