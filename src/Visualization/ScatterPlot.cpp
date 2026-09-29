#include "Visualization/ScatterPlot.h"
#include <QPainter>

ScatterPlot::ScatterPlot(QWidget* parent) : ChartWidget(parent) {}

void ScatterPlot::setPoints(const QVector<QPointF>& points) {
    m_points = points;
    update();
}

void ScatterPlot::setTrendLine(double slope, double intercept, const QString& info) {
    m_hasTrend = true;
    m_slope = slope;
    m_intercept = intercept;
    m_trendInfo = info;
    update();
}

void ScatterPlot::clearTrendLine() {
    m_hasTrend = false;
    m_trendInfo.clear();
    update();
}

void ScatterPlot::clearChart() {
    m_points.clear();
    clearTrendLine();
}

void ScatterPlot::dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const {
    xMin = xMax = m_points[0].x();
    yMin = yMax = m_points[0].y();
    for (const QPointF& p : m_points) {
        xMin = qMin(xMin, p.x());
        xMax = qMax(xMax, p.x());
        yMin = qMin(yMin, p.y());
        yMax = qMax(yMax, p.y());
    }
}

void ScatterPlot::drawContent(QPainter& painter, const QRectF& plot) {
    QColor dot = seriesColor(0);
    dot.setAlpha(m_points.size() > 1000 ? 70 : 150);   // transparent dots show density
    painter.setPen(Qt::NoPen);
    painter.setBrush(dot);
    for (const QPointF& p : m_points)
        painter.drawEllipse(toScreen(p.x(), p.y(), plot), 3, 3);

    if (m_hasTrend) {
        QPointF a = toScreen(m_xMin, m_slope * m_xMin + m_intercept, plot);
        QPointF b = toScreen(m_xMax, m_slope * m_xMax + m_intercept, plot);
        painter.setPen(QPen(QColor("#e8590c"), 2.5, Qt::DashLine));
        painter.drawLine(a, b);

        QRectF box(plot.left() + 10, plot.top() + 8, 260, 22);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 220));
        painter.drawRoundedRect(box, 4, 4);
        painter.setPen(QColor("#e8590c"));
        painter.drawText(box.adjusted(6, 0, 0, 0), Qt::AlignVCenter, m_trendInfo);
    }
}
