#ifndef PRISM_CHART_WIDGET_H
#define PRISM_CHART_WIDGET_H

#include <QWidget>
#include <QString>
#include <QColor>
#include <QRectF>

class QPainter;

// ---------------------------------------------------------------------------
// ChartWidget - ABSTRACT base class for every chart (drawn with QPainter).
//
// The base class does the common work in paintEvent():
//   background, title, axes, grid lines, tick numbers and axis labels.
// Each subclass only has to answer three questions (virtual functions):
//   hasData()     - is there anything to draw?
//   dataRange()   - what are the min/max x and y values?
//   drawContent() - draw the lines / bars / points inside the plot area.
//
//   ChartWidget
//     |- LineChart     |- BarChart     |- ScatterPlot     |- Histogram
// ---------------------------------------------------------------------------
class ChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChartWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setAxisLabels(const QString& xLabel, const QString& yLabel);

    static QColor seriesColor(int index);

protected:
    void paintEvent(QPaintEvent* event) override;

    virtual bool hasData() const = 0;
    virtual void dataRange(double& xMin, double& xMax, double& yMin, double& yMax) const = 0;
    virtual void drawContent(QPainter& painter, const QRectF& plot) = 0;

    // Subclasses can replace the numeric x axis (e.g. bar chart category names).
    virtual bool numericXAxis() const { return true; }
    virtual void drawCustomXAxis(QPainter& painter, const QRectF& plot);
    virtual int bottomMargin() const { return 48; }

    // Converts a data point to pixel coordinates inside the plot rectangle.
    QPointF toScreen(double x, double y, const QRectF& plot) const;

    // The (rounded) ranges actually used for the current paint.
    double m_xMin = 0, m_xMax = 1, m_yMin = 0, m_yMax = 1;
    QString m_title;
    QString m_xLabel;
    QString m_yLabel;

private:
    void drawAxes(QPainter& painter, const QRectF& plot);
    static double niceStep(double range, int targetTicks);
    static void niceRange(double& low, double& high);
};

#endif // PRISM_CHART_WIDGET_H
