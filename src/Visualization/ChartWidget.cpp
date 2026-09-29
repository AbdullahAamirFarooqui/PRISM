#include "Visualization/ChartWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

ChartWidget::ChartWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(320, 240);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ChartWidget::setTitle(const QString& title) {
    m_title = title;
    update();
}

void ChartWidget::setAxisLabels(const QString& xLabel, const QString& yLabel) {
    m_xLabel = xLabel;
    m_yLabel = yLabel;
    update();
}

QColor ChartWidget::seriesColor(int index) {
    static const QColor colors[] = {QColor("#5b4fcf"), QColor("#e8590c"), QColor("#2b8a3e"),
                                    QColor("#1971c2"), QColor("#c2255c"), QColor("#f08c00"),
                                    QColor("#0c8599"), QColor("#862e9c"), QColor("#5c940d"),
                                    QColor("#495057")};
    return colors[index % 10];
}

// Picks a "human" step size such as 1, 2, 5, 10, 20, 50 ...
double ChartWidget::niceStep(double range, int targetTicks) {
    if (range <= 0)
        return 1;
    double rough = range / targetTicks;
    double magnitude = std::pow(10.0, std::floor(std::log10(rough)));
    double residual = rough / magnitude;
    double nice = residual < 1.5 ? 1 : residual < 3 ? 2 : residual < 7 ? 5 : 10;
    return nice * magnitude;
}

void ChartWidget::niceRange(double& low, double& high) {
    if (low == high) {           // avoid a zero-height axis
        low -= 1;
        high += 1;
    }
    double step = niceStep(high - low, 6);
    low = std::floor(low / step) * step;
    high = std::ceil(high / step) * step;
}

QPointF ChartWidget::toScreen(double x, double y, const QRectF& plot) const {
    double px = plot.left() + (x - m_xMin) / (m_xMax - m_xMin) * plot.width();
    double py = plot.bottom() - (y - m_yMin) / (m_yMax - m_yMin) * plot.height();
    return QPointF(px, py);
}

void ChartWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    // Title
    QFont titleFont = font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    painter.setFont(titleFont);
    painter.setPen(QColor("#3b2c85"));
    painter.drawText(QRectF(0, 6, width(), 26), Qt::AlignCenter, m_title);
    painter.setFont(font());

    if (!hasData()) {
        painter.setPen(QColor("#999"));
        painter.drawText(rect(), Qt::AlignCenter, "No data to display.\nChoose columns above.");
        return;
    }

    dataRange(m_xMin, m_xMax, m_yMin, m_yMax);
    if (numericXAxis())
        niceRange(m_xMin, m_xMax);
    niceRange(m_yMin, m_yMax);

    QRectF plot(70, 40, width() - 90, height() - 40 - bottomMargin());
    if (plot.width() < 50 || plot.height() < 50)
        return;

    drawAxes(painter, plot);
    painter.save();
    painter.setClipRect(plot.adjusted(-6, -6, 6, 6));
    drawContent(painter, plot);          // <- the subclass draws here
    painter.restore();
}

void ChartWidget::drawAxes(QPainter& painter, const QRectF& plot) {
    QPen gridPen(QColor("#ecebf3"));
    QPen axisPen(QColor("#888"));
    QPen textPen(QColor("#555"));

    // horizontal grid + y numbers
    double yStep = niceStep(m_yMax - m_yMin, 6);
    for (double y = m_yMin; y <= m_yMax + yStep * 0.001; y += yStep) {
        QPointF p = toScreen(m_xMin, y, plot);
        painter.setPen(gridPen);
        painter.drawLine(QPointF(plot.left(), p.y()), QPointF(plot.right(), p.y()));
        painter.setPen(textPen);
        double shown = std::fabs(y) < yStep * 1e-6 ? 0 : y;
        painter.drawText(QRectF(0, p.y() - 8, plot.left() - 6, 16), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(shown, 'g', 6));
    }

    // vertical grid + x numbers
    if (numericXAxis()) {
        double xStep = niceStep(m_xMax - m_xMin, 7);
        for (double x = m_xMin; x <= m_xMax + xStep * 0.001; x += xStep) {
            QPointF p = toScreen(x, m_yMin, plot);
            painter.setPen(gridPen);
            painter.drawLine(QPointF(p.x(), plot.top()), QPointF(p.x(), plot.bottom()));
            painter.setPen(textPen);
            double shown = std::fabs(x) < xStep * 1e-6 ? 0 : x;
            painter.drawText(QRectF(p.x() - 40, plot.bottom() + 4, 80, 16), Qt::AlignCenter,
                             QString::number(shown, 'g', 6));
        }
    } else {
        drawCustomXAxis(painter, plot);
    }

    painter.setPen(axisPen);
    painter.drawLine(plot.bottomLeft(), plot.bottomRight());
    painter.drawLine(plot.bottomLeft(), plot.topLeft());

    // axis labels
    painter.setPen(QColor("#3b2c85"));
    painter.drawText(QRectF(plot.left(), height() - 20, plot.width(), 18), Qt::AlignCenter, m_xLabel);
    painter.save();
    painter.translate(14, plot.center().y());
    painter.rotate(-90);
    painter.drawText(QRectF(-plot.height() / 2, -9, plot.height(), 18), Qt::AlignCenter, m_yLabel);
    painter.restore();
}

void ChartWidget::drawCustomXAxis(QPainter&, const QRectF&) {}
