#include "Visualization/BarChart.h"
#include <QPainter>

BarChart::BarChart(QWidget* parent) : ChartWidget(parent) {}

void BarChart::setBars(const QStringList& labels, const QVector<double>& values) {
    m_labels = labels;
    m_values = values;
    update();
}

void BarChart::clearChart() {
    m_labels.clear();
    m_values.clear();
    update();
}

void BarChart::dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const {
    xMin = 0;
    xMax = m_values.size();
    yMin = 0;                       // bars always start from zero
    yMax = 0;
    for (double v : m_values) {
        yMin = qMin(yMin, v);
        yMax = qMax(yMax, v);
    }
    yMax *= 1.08;                   // a little space above the tallest bar for its label
}

void BarChart::drawContent(QPainter& painter, const QRectF& plot) {
    int n = m_values.size();
    double slot = plot.width() / n;
    double barWidth = slot * 0.7;
    QPointF zero = toScreen(0, 0, plot);

    for (int i = 0; i < n; ++i) {
        double left = plot.left() + i * slot + (slot - barWidth) / 2;
        QPointF top = toScreen(0, m_values[i], plot);
        QRectF bar(left, qMin(top.y(), zero.y()), barWidth, qAbs(zero.y() - top.y()));

        QColor color = n <= 10 ? seriesColor(i) : seriesColor(0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRect(bar);

        if (m_showValues && slot > 28) {
            painter.setPen(QColor("#333"));
            QString text = QString::number(m_values[i], 'g', 4);
            painter.drawText(QRectF(left - 20, bar.top() - 16, barWidth + 40, 14), Qt::AlignCenter, text);
        }
    }
}

void BarChart::drawCustomXAxis(QPainter& painter, const QRectF& plot) {
    int n = m_values.size();
    double slot = plot.width() / n;
    painter.setPen(QColor("#555"));
    for (int i = 0; i < n && i < m_labels.size(); ++i) {
        double centerX = plot.left() + i * slot + slot / 2;
        QString label = m_labels[i];
        if (label.length() > 14)
            label = label.left(13) + "...";
        // rotate labels so long names still fit
        painter.save();
        painter.translate(centerX, plot.bottom() + 6);
        painter.rotate(n > 6 ? 40 : 0);
        if (n > 6)
            painter.drawText(QRectF(0, -7, 100, 14), Qt::AlignLeft | Qt::AlignVCenter, label);
        else
            painter.drawText(QRectF(-slot / 2, 0, slot, 16), Qt::AlignCenter, label);
        painter.restore();
    }
}
