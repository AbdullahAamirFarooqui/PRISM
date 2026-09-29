#ifndef PRISM_MAIN_WINDOW_H
#define PRISM_MAIN_WINDOW_H

#include <QMainWindow>
#include <QVector>
#include "Data/Workspace.h"

class QTabWidget;
class QAction;
class QLabel;
class QMenu;
class BaseView;
class HistoryPanel;

// ---------------------------------------------------------------------------
// MainWindow - owns the Workspace and all screens.
//
// All views are stored as BaseView* (polymorphism): after any change the
// window just calls refreshIfNeeded() on the visible view, without knowing
// which concrete screen it is.
// ---------------------------------------------------------------------------
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openCsv();
    void openSample(const QString& fileName);
    void openProject();
    void saveProject();
    void exportCsv();
    void exportReport();
    void loadComparison();
    void closeComparison();
    void undo();
    void redo();
    void showAbout();
    void onDataModified();
    void onTabChanged(int index);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void addView(BaseView* view);
    void refreshAll();
    void updateActions();
    bool requireData();
    void loadCsvFile(const QString& path);
    QString sampleFolder() const;

    Workspace m_workspace;
    QTabWidget* m_tabs;
    QVector<BaseView*> m_views;
    HistoryPanel* m_history;
    QLabel* m_statusLabel;

    QAction* m_openAction;
    QAction* m_openProjectAction;
    QAction* m_saveAction;
    QAction* m_exportCsvAction;
    QAction* m_reportAction;
    QAction* m_compareAction;
    QAction* m_closeCompareAction;
    QAction* m_undoAction;
    QAction* m_redoAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
    QMenu* m_viewMenu;
};

#endif // PRISM_MAIN_WINDOW_H
