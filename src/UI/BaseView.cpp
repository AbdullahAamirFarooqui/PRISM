#include "UI/BaseView.h"
#include "UI/Style.h"
#include <QComboBox>

BaseView::BaseView(Workspace& workspace, QWidget* parent)
    : QWidget(parent), m_workspace(workspace) {
    setObjectName("page");
}

void BaseView::refreshIfNeeded() {
    if (m_seenVersion == m_workspace.version())
        return;
    m_seenVersion = m_workspace.version();
    refresh();
}

void BaseView::fillColumnCombo(QComboBox* combo, bool numericOnly, const QString& firstItem) {
    // Try to keep the same column selected after the refresh.
    QString previous = combo->currentText();
    combo->blockSignals(true);
    combo->clear();
    if (!firstItem.isEmpty())
        combo->addItem(firstItem, -1);

    const Dataset& data = m_workspace.data();
    for (int c = 0; c < data.columnCount(); ++c) {
        if (numericOnly && !data.isNumeric(c))
            continue;
        combo->addItem(Style::toQ(data.columnName(c)), c);
    }
    int index = combo->findText(previous);
    combo->setCurrentIndex(index >= 0 ? index : 0);
    combo->blockSignals(false);
}

int BaseView::selectedColumn(QComboBox* combo) const {
    if (combo->currentIndex() < 0)
        return -1;
    return combo->currentData().toInt();
}
