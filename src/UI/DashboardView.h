#ifndef PRISM_DASHBOARD_VIEW_H
#define PRISM_DASHBOARD_VIEW_H

#include "UI/BaseView.h"
#include "Data/BatchProcessor.h"

class QLabel;
class QTableWidget;
class QPushButton;
class QProgressBar;
class QPlainTextEdit;
class QTimer;
class QStackedWidget;
class QFrame;

// ---------------------------------------------------------------------------
// DashboardView - first screen:
//   - welcome page with quick-start buttons (when nothing is loaded)
//   - summary cards (rows, columns, missing, invalid, duplicates)
//   - data quality table per column
//   - batch pipeline: jobs go into a circular Queue and run one by one
// ---------------------------------------------------------------------------
class DashboardView : public BaseView {
    Q_OBJECT
public:
    explicit DashboardView(Workspace& workspace, QWidget* parent = nullptr);
    QString viewName() const override { return "Dashboard"; }

signals:
    void openCsvRequested();
    void openSampleRequested(const QString& fileName);

protected:
    void refresh() override;

private slots:
    void startBatch();
    void runNextJob();

private:
    QWidget* buildWelcomePage();
    QWidget* buildDashboardPage();
    QFrame* makeCard(const QString& caption, QLabel*& valueLabel);

    QStackedWidget* m_pages;
    QLabel* m_datasetTitle;
    QLabel* m_rowsValue;
    QLabel* m_columnsValue;
    QLabel* m_numericValue;
    QLabel* m_missingValue;
    QLabel* m_invalidValue;
    QLabel* m_duplicateValue;
    QLabel* m_comparisonInfo;
    QTableWidget* m_qualityTable;

    QPushButton* m_batchButton;
    QProgressBar* m_batchProgress;
    QLabel* m_batchStatus;
    QPlainTextEdit* m_batchOutput;
    QTimer* m_batchTimer;
    BatchProcessor m_batch;
};

#endif // PRISM_DASHBOARD_VIEW_H
