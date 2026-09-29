#ifndef PRISM_BASE_VIEW_H
#define PRISM_BASE_VIEW_H

#include <QWidget>
#include "Data/Workspace.h"

class QComboBox;

// ---------------------------------------------------------------------------
// BaseView - abstract parent class of every screen (tab) in the main window.
//
//  - Every view gets a reference to the shared Workspace.
//  - refresh() is PURE VIRTUAL: each screen decides how to redraw itself.
//  - refreshIfNeeded() only calls refresh() when the data really changed
//    (the Workspace version number went up), so hidden tabs stay cheap.
//  - When a view modifies the data it emits dataModified(); the MainWindow
//    then updates the history panel, undo buttons and the other views.
// ---------------------------------------------------------------------------
class BaseView : public QWidget {
    Q_OBJECT
public:
    explicit BaseView(Workspace& workspace, QWidget* parent = nullptr);

    void refreshIfNeeded();
    virtual QString viewName() const = 0;

signals:
    void dataModified();
    void statusMessage(const QString& message);

protected:
    virtual void refresh() = 0;

    // Fills a combo box with column names. The column index is stored as item data.
    void fillColumnCombo(QComboBox* combo, bool numericOnly, const QString& firstItem = QString());
    int selectedColumn(QComboBox* combo) const;

    Workspace& m_workspace;

private:
    int m_seenVersion = -1;
};

#endif // PRISM_BASE_VIEW_H
