#include "UI/HistoryPanel.h"
#include "UI/Style.h"
#include "Data/HistoryLog.h"
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>

HistoryPanel::HistoryPanel(QWidget* parent) : QWidget(parent) {
    m_list = new QListWidget;
    m_list->setWordWrap(true);
    m_list->setAlternatingRowColors(true);
    m_count = Style::makeHint("No operations yet.");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(m_count);
    layout->addWidget(m_list);
}

void HistoryPanel::showHistory(const HistoryLog& history) {
    m_list->clear();
    for (const LinkedList<HistoryEntry>::Node* n = history.entries().last(); n != nullptr; n = n->prev) {
        QString time = Style::toQ(n->data.timestamp).mid(11);     // only HH:MM:SS
        m_list->addItem(time + "   " + Style::toQ(n->data.action));
    }
    m_count->setText(QString("%1 operations (newest first)").arg(history.size()));
}
