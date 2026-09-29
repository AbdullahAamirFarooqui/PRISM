#include "Visualization/LineChart.h"
#include <QPainter>
#include <QPolygonF>

LineChart::LineChart(QWidget* parent) : ChartWidget(parent) {}

void LineChart::setSeries(const QVector<Series>& series) {
    m_series = series;
    update();
}

void LineChart::clearChart() {
    m_series.clear();
    update();
}

bool LineChart::hasData() const {
    for (const Series& s : m_series)
        if (!s.points.isEmpty())
            return true;
    return false;
}

void LineChart::dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const {
    bool first = true;
    for (const Series& s : m_series) {
        for (const QPointF& p : s.points) {
            if (first) {
                xMin = xMax = p.x();
                yMin = yMax = p.y();
                first = false;
            }
            xMin = qMin(xMin, p.x());
            xMax = qMax(xMax, p.x());
            yMin = qMin(yMin, p.y());
            yMax = qMax(yMax, p.y());
        }
    }
}

void LineChart::drawContent(QPainter& painter, const QRectF& plot) {
    for (int s = 0; s < m_series.size(); ++s) {
        const QVector<QPointF>& points = m_series[s].points;
        // Very long series: skip points so we draw about 2 per pixel.
        int step = qMax(1, static_cast<int>(points.size() / (plot.width() * 2)));
        QPolygonF line;
        for (int i = 0; i < points.size(); i += step)
            line << toScreen(points[i].x(), points[i].y(), plot);

        QColor color = seriesColor(s);
        painter.setPen(QPen(color, 2));
        painter.drawPolyline(line);

        if (points.size() <= 60) {           // few points -> draw dots too
            painter.setBrush(Qt::white);
            for (const QPointF& p : line)
                painter.drawEllipse(p, 3.5, 3.5);
        }
    }
    if (m_series.size() > 1)
        drawLegend(painter, plot);
}

void LineChart::drawLegend(QPainter& painter, const QRectF& plot) {
    int rowHeight = 18;
    int width = 0;
    for (const Series& s : m_series)
        width = qMax(width, painter.fontMetrics().horizontalAdvance(s.name));
    QRectF box(plot.right() - width - 44, plot.top() + 6, width + 38, rowHeight * m_series.size() + 8);

    painter.setPen(QColor("#dcdbe8"));
    painter.setBrush(QColor(255, 255, 255, 230));
    painter.drawRoundedRect(box, 4, 4);
    for (int s = 0; s < m_series.size(); ++s) {
        double y = box.top() + 4 + s * rowHeight + rowHeight / 2.0;
        painter.setPen(QPen(seriesColor(s), 3));
        painter.drawLine(QPointF(box.left() + 8, y), QPointF(box.left() + 24, y));
        painter.setPen(QColor("#333"));
        painter.drawText(QRectF(box.left() + 30, y - 9, width + 8, 18), Qt::AlignVCenter, m_series[s].name);
    }
}
