#include "UI/MainWindow.h"
#include "UI/Style.h"
#include "UI/BaseView.h"
#include "UI/DashboardView.h"
#include "UI/DatasetView.h"
#include "UI/StatisticsView.h"
#include "UI/ChartsView.h"
#include "UI/CompareView.h"
#include "UI/AlgorithmLab.h"
#include "UI/GraphView.h"
#include "UI/HistoryPanel.h"
#include <QTabWidget>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QKeySequence>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("PRISM - Scientific Data Analyzer");
    resize(1360, 820);

    m_tabs = new QTabWidget;
    setCentralWidget(m_tabs);

    DashboardView* dashboard = new DashboardView(m_workspace);
    connect(dashboard, &DashboardView::openCsvRequested, this, &MainWindow::openCsv);
    connect(dashboard, &DashboardView::openSampleRequested, this, &MainWindow::openSample);
    CompareView* compare = new CompareView(m_workspace);
    connect(compare, &CompareView::loadComparisonRequested, this, &MainWindow::loadComparison);

    addView(dashboard);
    addView(new DatasetView(m_workspace));
    addView(new StatisticsView(m_workspace));
    addView(new ChartsView(m_workspace));
    addView(compare);
    addView(new AlgorithmLab(m_workspace));
    addView(new GraphView(m_workspace));
    connect(m_tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // History panel on the right side
    m_history = new HistoryPanel;
    QDockWidget* dock = new QDockWidget("History (linked list)", this);
    dock->setWidget(m_history);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    dock->setMinimumWidth(260);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_statusLabel = new QLabel;
    statusBar()->addPermanentWidget(m_statusLabel);

    createActions();
    createMenus();
    createToolBar();
    m_viewMenu->addAction(dock->toggleViewAction());
    refreshAll();
}

void MainWindow::addView(BaseView* view) {
    m_views.append(view);
    m_tabs->addTab(view, view->viewName());      // virtual call
    connect(view, &BaseView::dataModified, this, &MainWindow::onDataModified);
    connect(view, &BaseView::statusMessage, this, [this](const QString& text) {
        statusBar()->showMessage(text, 5000);
    });
}

// --------------------------------------------------------------- actions ---

void MainWindow::createActions() {
    m_openAction = new QAction("&Open CSV...", this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openCsv);

    m_openProjectAction = new QAction("Open &Project (.sda)...", this);
    m_openProjectAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(m_openProjectAction, &QAction::triggered, this, &MainWindow::openProject);

    m_saveAction = new QAction("&Save Project (.sda)...", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveProject);

    m_exportCsvAction = new QAction("Export cleaned &CSV...", this);
    connect(m_exportCsvAction, &QAction::triggered, this, &MainWindow::exportCsv);

    m_reportAction = new QAction("Export &Report (HTML)...", this);
    m_reportAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(m_reportAction, &QAction::triggered, this, &MainWindow::exportReport);

    m_compareAction = new QAction("Load &comparison dataset...", this);
    connect(m_compareAction, &QAction::triggered, this, &MainWindow::loadComparison);

    m_closeCompareAction = new QAction("Close comparison dataset", this);
    connect(m_closeCompareAction, &QAction::triggered, this, &MainWindow::closeComparison);

    m_undoAction = new QAction("&Undo", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::undo);

    m_redoAction = new QAction("&Redo", this);
    m_redoAction->setShortcut(QKeySequence("Ctrl+Y"));
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::redo);

    m_exitAction = new QAction("E&xit", this);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    m_aboutAction = new QAction("&About PRISM", this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus() {
    QMenu* file = menuBar()->addMenu("&File");
    file->addAction(m_openAction);
    QMenu* samples = file->addMenu("Open &sample dataset");
    QDir folder(sampleFolder());
    for (const QString& name : folder.entryList({"*.csv"}, QDir::Files, QDir::Name))
        samples->addAction(name, this, [this, name]() { openSample(name); });
    if (samples->isEmpty())
        samples->addAction("(no data folder found)")->setEnabled(false);
    file->addAction(m_openProjectAction);
    file->addSeparator();
    file->addAction(m_saveAction);
    file->addAction(m_exportCsvAction);
    file->addAction(m_reportAction);
    file->addSeparator();
    file->addAction(m_compareAction);
    file->addAction(m_closeCompareAction);
    file->addSeparator();
    file->addAction(m_exitAction);

    QMenu* edit = menuBar()->addMenu("&Edit");
    edit->addAction(m_undoAction);
    edit->addAction(m_redoAction);

    m_viewMenu = menuBar()->addMenu("&View");

    QMenu* help = menuBar()->addMenu("&Help");
    help->addAction(m_aboutAction);
}

void MainWindow::createToolBar() {
    QToolBar* bar = addToolBar("Main");
    bar->setMovable(false);
    bar->addAction(m_openAction);
    bar->addAction(m_saveAction);
    bar->addSeparator();
    bar->addAction(m_undoAction);
    bar->addAction(m_redoAction);
    bar->addSeparator();
    bar->addAction(m_compareAction);
    bar->addAction(m_reportAction);
}

// ----------------------------------------------------------------- files ---

QString MainWindow::sampleFolder() const {
    // "data" next to the .exe (copied there by CMake), or the project folder one level up.
    QString exeDir = QCoreApplication::applicationDirPath();
    for (const QString& candidate : {exeDir + "/data", exeDir + "/../data", exeDir + "/../../data"})
        if (QDir(candidate).exists())
            return QDir(candidate).absolutePath();
    return exeDir + "/data";
}

void MainWindow::loadCsvFile(const QString& path) {
    std::string error;
    if (!m_workspace.openCSV(Style::toStdPath(path), error)) {
        QMessageBox::warning(this, "Open CSV", Style::toQ(error));
        return;
    }
    statusBar()->showMessage("Loaded " + QFileInfo(path).fileName(), 5000);
    refreshAll();
}

void MainWindow::openCsv() {
    QString path = QFileDialog::getOpenFileName(this, "Open CSV dataset", sampleFolder(),
                                                "CSV files (*.csv *.txt);;All files (*)");
    if (!path.isEmpty())
        loadCsvFile(path);
}

void MainWindow::openSample(const QString& fileName) {
    QString path = sampleFolder() + "/" + fileName;
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, "Sample dataset", "Could not find " + path);
        return;
    }
    loadCsvFile(path);
}

void MainWindow::openProject() {
    QString path = QFileDialog::getOpenFileName(this, "Open PRISM project", QString(),
                                                "PRISM project (*.sda)");
    if (path.isEmpty()) return;
    std::string error;
    if (!m_workspace.openProject(Style::toStdPath(path), error)) {
        QMessageBox::warning(this, "Open project", Style::toQ(error));
        return;
    }
    refreshAll();
}

bool MainWindow::requireData() {
    if (m_workspace.hasData())
        return true;
    QMessageBox::information(this, "PRISM", "Please open a dataset first.");
    return false;
}

void MainWindow::saveProject() {
    if (!requireData()) return;
    QString suggested = Style::toQ(m_workspace.data().name()) + ".sda";
    QString path = QFileDialog::getSaveFileName(this, "Save PRISM project", suggested, "PRISM project (*.sda)");
    if (path.isEmpty()) return;
    std::string error;
    if (!m_workspace.saveProject(Style::toStdPath(path), error))
        QMessageBox::warning(this, "Save project", Style::toQ(error));
    else
        statusBar()->showMessage("Project saved", 5000);
    onDataModified();
}

void MainWindow::exportCsv() {
    if (!requireData()) return;
    QString suggested = Style::toQ(m_workspace.data().name()) + "_clean.csv";
    QString path = QFileDialog::getSaveFileName(this, "Export CSV", suggested, "CSV files (*.csv)");
    if (path.isEmpty()) return;
    std::string error;
    if (!m_workspace.exportCSV(Style::toStdPath(path), error))
        QMessageBox::warning(this, "Export CSV", Style::toQ(error));
    onDataModified();
}

void MainWindow::exportReport() {
    if (!requireData()) return;
    QString suggested = Style::toQ(m_workspace.data().name()) + "_report.html";
    QString path = QFileDialog::getSaveFileName(this, "Export analysis report", suggested, "HTML report (*.html)");
    if (path.isEmpty()) return;
    std::string error;
    if (!m_workspace.exportReport(Style::toStdPath(path), error)) {
        QMessageBox::warning(this, "Report", Style::toQ(error));
        return;
    }
    onDataModified();
    if (QMessageBox::question(this, "Report", "Report saved. Open it in your browser now?") == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::loadComparison() {
    if (!requireData()) return;
    QString path = QFileDialog::getOpenFileName(this, "Open second dataset for comparison", sampleFolder(),
                                                "CSV files (*.csv *.txt)");
    if (path.isEmpty()) return;
    std::string error;
    if (!m_workspace.openComparisonCSV(Style::toStdPath(path), error)) {
        QMessageBox::warning(this, "Comparison", Style::toQ(error));
        return;
    }
    refreshAll();
    m_tabs->setCurrentIndex(4);          // jump to the Compare tab
}

void MainWindow::closeComparison() {
    m_workspace.closeComparison();
    refreshAll();
}

// ------------------------------------------------------------- undo/redo ---

void MainWindow::undo() {
    std::string description;
    if (m_workspace.undo(description))
        statusBar()->showMessage("Undone: " + Style::toQ(description), 5000);
    onDataModified();
}

void MainWindow::redo() {
    std::string description;
    if (m_workspace.redo(description))
        statusBar()->showMessage("Redone: " + Style::toQ(description), 5000);
    onDataModified();
}

// --------------------------------------------------------------- refresh ---

void MainWindow::onDataModified() {
    BaseView* current = qobject_cast<BaseView*>(m_tabs->currentWidget());
    if (current)
        current->refreshIfNeeded();         // other tabs refresh when they are opened
    m_history->showHistory(m_workspace.history());
    updateActions();
}

void MainWindow::onTabChanged(int index) {
    if (index >= 0 && index < m_views.size())
        m_views[index]->refreshIfNeeded();
}

void MainWindow::refreshAll() {
    onDataModified();
}

void MainWindow::updateActions() {
    bool hasData = m_workspace.hasData();
    const UndoManager& undo = m_workspace.undoManager();
    m_undoAction->setEnabled(undo.canUndo());
    m_redoAction->setEnabled(undo.canRedo());
    m_undoAction->setText(undo.canUndo() ? "Undo " + Style::toQ(undo.nextUndoText()) : "Undo");
    m_redoAction->setText(undo.canRedo() ? "Redo " + Style::toQ(undo.nextRedoText()) : "Redo");
    m_saveAction->setEnabled(hasData);
    m_exportCsvAction->setEnabled(hasData);
    m_reportAction->setEnabled(hasData);
    m_compareAction->setEnabled(hasData);
    m_closeCompareAction->setEnabled(m_workspace.hasComparison());
    for (int i = 1; i < m_tabs->count(); ++i)
        m_tabs->setTabEnabled(i, hasData);

    if (hasData) {
        const Dataset& d = m_workspace.data();
        m_statusLabel->setText(QString("%1  |  %2 rows x %3 columns  |  undo: %4  redo: %5")
                                   .arg(Style::toQ(d.name())).arg(d.rowCount()).arg(d.columnCount())
                                   .arg(undo.undoCount()).arg(undo.redoCount()));
    } else {
        m_statusLabel->setText("No dataset loaded");
    }
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "About PRISM",
        "<h3>PRISM - Scientific Data Analyzer</h3>"
        "<p>Data Structures (CL-2001) semester project, FAST NUCES Karachi.</p>"
        "<p>Every data structure is implemented from scratch in C++17:</p>"
        "<ul><li>DynamicArray - dataset storage</li><li>LinkedList - history log, graph adjacency</li>"
        "<li>Stack - undo / redo</li><li>Queue (circular) - batch pipeline, BFS</li>"
        "<li>BST - column index, range queries</li><li>Heap - Top-K ranking, heap sort</li>"
        "<li>HashTable - duplicate detection, exact lookup, grouping</li>"
        "<li>Graph - correlation network with BFS / DFS</li></ul>"
        "<p>The Qt library is used only for the graphical interface.</p>");
}
