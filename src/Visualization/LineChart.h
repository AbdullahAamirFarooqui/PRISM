#ifndef PRISM_LINE_CHART_H
#define PRISM_LINE_CHART_H

#include "Visualization/ChartWidget.h"
#include <QVector>
#include <QPointF>

// Line chart with one or more series (several series = comparison / grouping).
class LineChart : public ChartWidget {
    Q_OBJECT
public:
    struct Series {
        QString name;
        QVector<QPointF> points;   // should be sorted by x
    };

    explicit LineChart(QWidget* parent = nullptr);

    void setSeries(const QVector<Series>& series);
    void clearChart();

protected:
    bool hasData() const override;
    void dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const override;
    void drawContent(QPainter& painter, const QRectF& plot) override;

private:
    QVector<Series> m_series;
    void drawLegend(QPainter& painter, const QRectF& plot);
};

#endif // PRISM_LINE_CHART_H
