#ifndef PRISM_HISTOGRAM_H
#define PRISM_HISTOGRAM_H

#include "Visualization/ChartWidget.h"
#include <QVector>

// Histogram: touching bars, one per value range (bin). The x axis is numeric.
class Histogram : public ChartWidget {
    Q_OBJECT
public:
    struct Bin {
        double from = 0;
        double to = 0;
        int count = 0;
    };

    explicit Histogram(QWidget* parent = nullptr);

    void setBins(const QVector<Bin>& bins);
    void clearChart();

protected:
    bool hasData() const override { return !m_bins.isEmpty(); }
    void dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const override;
    void drawContent(QPainter& painter, const QRectF& plot) override;

private:
    QVector<Bin> m_bins;
};

#endif // PRISM_HISTOGRAM_H
