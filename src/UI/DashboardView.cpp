#include "UI/DashboardView.h"
#include "UI/Style.h"
#include "Data/DataValidator.h"
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QTimer>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>

DashboardView::DashboardView(Workspace& workspace, QWidget* parent) : BaseView(workspace, parent) {
    m_pages = new QStackedWidget(this);
    m_pages->addWidget(buildWelcomePage());
    m_pages->addWidget(buildDashboardPage());

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pages);

    m_batchTimer = new QTimer(this);
    m_batchTimer->setInterval(250);          // one job every 250 ms so the user sees progress
    connect(m_batchTimer, &QTimer::timeout, this, &DashboardView::runNextJob);
}

QWidget* DashboardView::buildWelcomePage() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->addStretch();

    QLabel* logo = new QLabel("PRISM");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 54px; font-weight: bold; color: #5b4fcf; letter-spacing: 6px;");
    QLabel* subtitle = new QLabel("Scientific Data Analyzer");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 18px; color: #555;");
    QLabel* info = new QLabel("Import a CSV file to clean, index, analyse and visualise it.\n"
                              "Every data structure and algorithm is implemented from scratch in C++.");
    info->setAlignment(Qt::AlignCenter);
    info->setStyleSheet("color: #777; margin: 10px;");
    layout->addWidget(logo);
    layout->addWidget(subtitle);
    layout->addWidget(info);

    QPushButton* openButton = new QPushButton("Open CSV file...");
    openButton->setMinimumWidth(220);
    connect(openButton, &QPushButton::clicked, this, &DashboardView::openCsvRequested);
    QHBoxLayout* openRow = new QHBoxLayout;
    openRow->addStretch();
    openRow->addWidget(openButton);
    openRow->addStretch();
    layout->addLayout(openRow);

    QLabel* samplesLabel = new QLabel("...or try a sample dataset:");
    samplesLabel->setAlignment(Qt::AlignCenter);
    samplesLabel->setStyleSheet("color: #777; margin-top: 18px;");
    layout->addWidget(samplesLabel);

    QHBoxLayout* samples = new QHBoxLayout;
    samples->addStretch();
    const char* files[] = {"weather_messy.csv", "weather_2025.csv", "earthquakes.csv",
                           "population.csv", "rainfall_messy.csv"};
    for (const char* file : files) {
        QPushButton* button = new QPushButton(QString(file).replace(".csv", ""));
        button->setProperty("secondary", true);
        QString name = file;
        connect(button, &QPushButton::clicked, this, [this, name]() { emit openSampleRequested(name); });
        samples->addWidget(button);
    }
    samples->addStretch();
    layout->addLayout(samples);
    layout->addStretch();
    return page;
}

QFrame* DashboardView::makeCard(const QString& caption, QLabel*& valueLabel) {
    QFrame* card = new QFrame;
    card->setObjectName("card");
    QVBoxLayout* layout = new QVBoxLayout(card);
    valueLabel = new QLabel("0");
    valueLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #3b2c85;");
    QLabel* captionLabel = new QLabel(caption);
    captionLabel->setStyleSheet("color: #777;");
    layout->addWidget(valueLabel);
    layout->addWidget(captionLabel);
    return card;
}

QWidget* DashboardView::buildDashboardPage() {
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);

    m_datasetTitle = Style::makeTitle("Dataset");
    layout->addWidget(m_datasetTitle);

    QHBoxLayout* cards = new QHBoxLayout;
    cards->addWidget(makeCard("rows", m_rowsValue));
    cards->addWidget(makeCard("columns", m_columnsValue));
    cards->addWidget(makeCard("numeric columns", m_numericValue));
    cards->addWidget(makeCard("missing cells", m_missingValue));
    cards->addWidget(makeCard("invalid cells", m_invalidValue));
    cards->addWidget(makeCard("duplicate rows", m_duplicateValue));
    layout->addLayout(cards);

    m_comparisonInfo = Style::makeHint("");
    layout->addWidget(m_comparisonInfo);

    QHBoxLayout* bottom = new QHBoxLayout;

    // --- data quality table ---
    QGroupBox* qualityBox = new QGroupBox("Data quality per column (DataValidator)");
    QVBoxLayout* qualityLayout = new QVBoxLayout(qualityBox);
    m_qualityTable = new QTableWidget;
    m_qualityTable->setColumnCount(6);
    m_qualityTable->setHorizontalHeaderLabels({"Column", "Type", "Valid", "Missing", "Invalid", "Complete"});
    m_qualityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_qualityTable->verticalHeader()->setVisible(false);
    m_qualityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    qualityLayout->addWidget(m_qualityTable);
    qualityLayout->addWidget(Style::makeHint(
        "Type inference: a column is numeric when at least 70% of its filled cells are numbers. "
        "Duplicate rows are found with the custom hash table in O(n)."));
    bottom->addWidget(qualityBox, 3);

    // --- batch pipeline ---
    QGroupBox* batchBox = new QGroupBox("Batch analysis pipeline (circular Queue, FIFO)");
    QVBoxLayout* batchLayout = new QVBoxLayout(batchBox);
    batchLayout->addWidget(Style::makeHint(
        "Queues a validation job, one statistics job per numeric column, a correlation job and a "
        "Top-K job. Jobs are dequeued and run one after another."));
    m_batchButton = new QPushButton("Run full analysis");
    connect(m_batchButton, &QPushButton::clicked, this, &DashboardView::startBatch);
    batchLayout->addWidget(m_batchButton);
    m_batchProgress = new QProgressBar;
    m_batchProgress->setValue(0);
    batchLayout->addWidget(m_batchProgress);
    m_batchStatus = new QLabel("Idle");
    batchLayout->addWidget(m_batchStatus);
    m_batchOutput = new QPlainTextEdit;
    m_batchOutput->setReadOnly(true);
    m_batchOutput->setStyleSheet("font-family: Consolas, monospace; font-size: 12px;");
    batchLayout->addWidget(m_batchOutput);
    bottom->addWidget(batchBox, 2);

    layout->addLayout(bottom, 1);
    return page;
}

void DashboardView::refresh() {
    if (!m_workspace.hasData()) {
        m_pages->setCurrentIndex(0);
        return;
    }
    m_pages->setCurrentIndex(1);

    const Dataset& data = m_workspace.data();
    ValidationReport report = DataValidator::validate(data);
    m_datasetTitle->setText("Dataset: " + Style::toQ(data.name()));
    m_rowsValue->setText(QString::number(report.rows));
    m_columnsValue->setText(QString::number(report.columns));
    m_numericValue->setText(QString::number(report.numericColumns));
    m_missingValue->setText(QString::number(report.totalMissing));
    m_invalidValue->setText(QString::number(report.totalInvalid));
    m_duplicateValue->setText(QString::number(report.duplicateRows));

    if (m_workspace.hasComparison())
        m_comparisonInfo->setText("Comparison dataset loaded: " + Style::toQ(m_workspace.comparison().name()) +
                                  " (" + QString::number(m_workspace.comparison().rowCount()) +
                                  " rows) - see the Compare tab.");
    else
        m_comparisonInfo->setText("Source: " + Style::toQ(data.sourcePath()));

    m_qualityTable->setRowCount(report.columnReports.size());
    for (int i = 0; i < report.columnReports.size(); ++i) {
        const ColumnReport& c = report.columnReports[i];
        double complete = report.rows > 0 ? 100.0 * c.valid / report.rows : 0;
        QStringList cells = {Style::toQ(c.name), c.type == ColumnType::Numeric ? "Numeric" : "Text",
                             QString::number(c.valid), QString::number(c.missing),
                             QString::number(c.invalid), QString::number(complete, 'f', 1) + " %"};
        for (int k = 0; k < cells.size(); ++k) {
            QTableWidgetItem* item = new QTableWidgetItem(cells[k]);
            if (k == 3 && c.missing > 0) item->setBackground(Style::MissingCell);
            if (k == 4 && c.invalid > 0) item->setBackground(Style::InvalidCell);
            m_qualityTable->setItem(i, k, item);
        }
    }
}

void DashboardView::startBatch() {
    if (!m_workspace.hasData() || m_batch.hasPending())
        return;
    m_batch.addFullAnalysis(m_workspace.data());
    m_batchOutput->clear();
    m_batchOutput->appendPlainText("Queued " + QString::number(m_batch.pendingCount()) + " jobs.");
    m_batchProgress->setMaximum(m_batch.totalJobs());
    m_batchProgress->setValue(0);
    m_batchButton->setEnabled(false);
    m_batchTimer->start();
}

void DashboardView::runNextJob() {
    if (!m_batch.hasPending()) {
        m_batchTimer->stop();
        return;
    }
    int total = m_batch.totalJobs();
    std::string jobName;
    std::string output = m_batch.runNext(m_workspace.data(), jobName);
    int done = total - m_batch.pendingCount();

    m_batchOutput->appendPlainText(QString("[%1/%2] %3\n      %4").arg(done).arg(total)
                                   .arg(Style::toQ(jobName), Style::toQ(output)));
    m_batchProgress->setValue(done);

    if (m_batch.hasPending()) {
        m_batchStatus->setText("Next: " + Style::toQ(m_batch.nextJobName()) + "   (" +
                               QString::number(m_batch.pendingCount()) + " waiting in queue)");
    } else {
        m_batchTimer->stop();
        m_batchStatus->setText("Finished - all jobs done.");
        m_batchButton->setEnabled(true);
        m_workspace.log("Ran batch analysis pipeline (" + std::to_string(total) + " jobs)");
        emit statusMessage("Batch analysis finished");
        emit dataModified();                    // updates the history panel
    }
}
