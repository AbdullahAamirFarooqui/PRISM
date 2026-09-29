#include "Visualization/Histogram.h"
#include <QPainter>

Histogram::Histogram(QWidget* parent) : ChartWidget(parent) {}

void Histogram::setBins(const QVector<Bin>& bins) {
    m_bins = bins;
    update();
}

void Histogram::clearChart() {
    m_bins.clear();
    update();
}

void Histogram::dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const {
    xMin = m_bins.first().from;
    xMax = m_bins.last().to;
    yMin = 0;
    yMax = 0;
    for (const Bin& b : m_bins)
        yMax = qMax(yMax, static_cast<double>(b.count));
    yMax *= 1.08;                   // space for the count labels
}

void Histogram::drawContent(QPainter& painter, const QRectF& plot) {
    QColor fill = seriesColor(0);
    fill.setAlpha(190);
    for (const Bin& b : m_bins) {
        QPointF topLeft = toScreen(b.from, b.count, plot);
        QPointF bottomRight = toScreen(b.to, 0, plot);
        QRectF bar(topLeft, bottomRight);
        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(fill);
        painter.drawRect(bar);
        if (bar.width() > 24 && b.count > 0) {
            painter.setPen(QColor("#333"));
            painter.drawText(QRectF(bar.left(), bar.top() - 15, bar.width(), 14), Qt::AlignCenter,
                             QString::number(b.count));
        }
    }
}
