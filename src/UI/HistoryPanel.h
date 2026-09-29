#ifndef PRISM_HISTORY_PANEL_H
#define PRISM_HISTORY_PANEL_H

#include <QWidget>

class QListWidget;
class QLabel;
class HistoryLog;

// ---------------------------------------------------------------------------
// HistoryPanel - shows the HistoryLog (a doubly linked list).
// We walk the list BACKWARDS (tail -> prev -> prev ...) so the newest
// operation is shown on top - that is only cheap because the list is doubly linked.
// ---------------------------------------------------------------------------
class HistoryPanel : public QWidget {
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget* parent = nullptr);
    void showHistory(const HistoryLog& history);

private:
    QListWidget* m_list;
    QLabel* m_count;
};

#endif // PRISM_HISTORY_PANEL_H
