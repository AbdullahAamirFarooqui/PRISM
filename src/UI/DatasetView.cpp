#include "UI/DatasetView.h"
#include "UI/DatasetTableModel.h"
#include "UI/Style.h"
#include "Data/DataCleaner.h"
#include "Data/DataValidator.h"
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QSet>
#include <algorithm>

DatasetView::DatasetView(Workspace& workspace, QWidget* parent) : BaseView(workspace, parent) {
    m_model = new DatasetTableModel(m_workspace.data(), this);
    m_table = new QTableView;
    m_table->setModel(m_model);
    m_table->setAlternatingRowColors(false);
    m_table->horizontalHeader()->setDefaultSectionSize(120);
    m_table->verticalHeader()->setDefaultSectionSize(24);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_summary = new QLabel;
    QLabel* legend = new QLabel(
        "<span style='background:#ffd6d6'>&nbsp;missing&nbsp;</span> "
        "<span style='background:#ffe2b8'>&nbsp;invalid&nbsp;</span> "
        "<span style='background:#e6dcff'>&nbsp;duplicate row&nbsp;</span> "
        "<span style='background:#fff3a3'>&nbsp;search match&nbsp;</span>");

    QWidget* left = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* top = new QHBoxLayout;
    top->addWidget(m_summary);
    top->addStretch();
    top->addWidget(legend);
    leftLayout->addLayout(top);
    leftLayout->addWidget(m_table);

    QScrollArea* scroll = new QScrollArea;
    scroll->setWidget(buildToolPanel());
    scroll->setWidgetResizable(true);
    scroll->setFixedWidth(360);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(left, 1);
    layout->addWidget(scroll);
}

static QPushButton* makeButton(const QString& text, bool secondary = false) {
    QPushButton* b = new QPushButton(text);
    if (secondary)
        b->setProperty("secondary", true);
    return b;
}

QWidget* DatasetView::buildToolPanel() {
    QWidget* panel = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(4, 0, 4, 0);

    // ---------------- cleaning ----------------
    QGroupBox* clean = new QGroupBox("Cleaning (undoable)");
    QVBoxLayout* cleanLayout = new QVBoxLayout(clean);
    QPushButton* dupButton = makeButton("Remove duplicate rows");
    connect(dupButton, &QPushButton::clicked, this, &DatasetView::removeDuplicates);
    cleanLayout->addWidget(dupButton);

    QFormLayout* form = new QFormLayout;
    m_cleanColumn = new QComboBox;
    m_strategy = new QComboBox;
    m_strategy->addItems({"Replace with mean", "Replace with median", "Replace with zero", "Remove rows"});
    form->addRow("Column:", m_cleanColumn);
    form->addRow("Strategy:", m_strategy);
    cleanLayout->addLayout(form);

    QPushButton* missingButton = makeButton("Handle missing values");
    QPushButton* invalidButton = makeButton("Fix invalid values");
    QPushButton* trimButton = makeButton("Trim whitespace", true);
    connect(missingButton, &QPushButton::clicked, this, &DatasetView::handleMissing);
    connect(invalidButton, &QPushButton::clicked, this, &DatasetView::fixInvalid);
    connect(trimButton, &QPushButton::clicked, this, &DatasetView::trimWhitespace);
    cleanLayout->addWidget(missingButton);
    cleanLayout->addWidget(invalidButton);
    cleanLayout->addWidget(trimButton);
    cleanLayout->addWidget(Style::makeHint(
        "Fix invalid first tries to repair values like \"1,200\" or \"23.5C\"; the rest are "
        "replaced with the strategy. Text columns are filled with their most frequent value."));
    layout->addWidget(clean);

    // ---------------- columns ----------------
    QGroupBox* columns = new QGroupBox("Columns");
    QFormLayout* columnForm = new QFormLayout(columns);
    m_editColumn = new QComboBox;
    m_newName = new QLineEdit;
    m_newName->setPlaceholderText("new name");
    QPushButton* renameButton = makeButton("Rename");
    QPushButton* removeButton = makeButton("Remove column", true);
    connect(renameButton, &QPushButton::clicked, this, &DatasetView::renameColumn);
    connect(removeButton, &QPushButton::clicked, this, &DatasetView::removeColumn);
    columnForm->addRow("Column:", m_editColumn);
    columnForm->addRow("Name:", m_newName);
    QHBoxLayout* columnButtons = new QHBoxLayout;
    columnButtons->addWidget(renameButton);
    columnButtons->addWidget(removeButton);
    columnForm->addRow(columnButtons);
    layout->addWidget(columns);

    // ---------------- search & index ----------------
    QGroupBox* search = new QGroupBox("Search && Index");
    QVBoxLayout* searchLayout = new QVBoxLayout(search);
    QFormLayout* searchForm = new QFormLayout;
    m_searchColumn = new QComboBox;
    searchForm->addRow("Column:", m_searchColumn);
    m_exactValue = new QLineEdit;
    m_exactValue->setPlaceholderText("exact value, e.g. Karachi");
    searchForm->addRow("Value:", m_exactValue);
    searchLayout->addLayout(searchForm);
    QPushButton* exactButton = makeButton("Find exact (hash table)");
    connect(exactButton, &QPushButton::clicked, this, &DatasetView::searchExact);
    connect(m_exactValue, &QLineEdit::returnPressed, this, &DatasetView::searchExact);
    searchLayout->addWidget(exactButton);

    QHBoxLayout* rangeRow = new QHBoxLayout;
    m_rangeLow = new QDoubleSpinBox;
    m_rangeHigh = new QDoubleSpinBox;
    for (QDoubleSpinBox* box : {m_rangeLow, m_rangeHigh}) {
        box->setRange(-1e12, 1e12);
        box->setDecimals(3);
        box->setMinimumWidth(70);          // a huge range would otherwise make the box very wide
    }
    rangeRow->addWidget(new QLabel("From"));
    rangeRow->addWidget(m_rangeLow, 1);
    rangeRow->addWidget(new QLabel("to"));
    rangeRow->addWidget(m_rangeHigh, 1);
    searchLayout->addLayout(rangeRow);
    QPushButton* rangeButton = makeButton("Find range (BST)");
    connect(rangeButton, &QPushButton::clicked, this, &DatasetView::searchRange);
    searchLayout->addWidget(rangeButton);

    m_onlyMatches = new QCheckBox("Show only matching rows");
    m_onlyMatches->setChecked(true);
    searchLayout->addWidget(m_onlyMatches);
    QPushButton* clearButton = makeButton("Clear search", true);
    connect(clearButton, &QPushButton::clicked, this, &DatasetView::clearSearch);
    searchLayout->addWidget(clearButton);

    m_searchResult = new QLabel;
    m_searchResult->setWordWrap(true);
    m_searchResult->setStyleSheet("color:#3b2c85; font-weight:bold;");
    searchLayout->addWidget(m_searchResult);
    m_indexInfo = Style::makeHint("");
    searchLayout->addWidget(m_indexInfo);
    layout->addWidget(search);

    // When the search column changes, pre-fill the range with its min and max.
    connect(m_searchColumn, &QComboBox::currentIndexChanged, this, [this]() {
        int col = selectedColumn(m_searchColumn);
        if (col >= 0 && ensureIndex(col) && m_index.hasTree()) {
            m_rangeLow->setValue(m_index.tree().minKey());
            m_rangeHigh->setValue(m_index.tree().maxKey());
        }
        updateIndexInfo();
    });

    // Long column names must not make the panel wider than the scroll area.
    for (QComboBox* combo : panel->findChildren<QComboBox*>()) {
        combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        combo->setMinimumContentsLength(8);
    }
    layout->addStretch();
    return panel;
}

void DatasetView::refresh() {
    const Dataset& data = m_workspace.data();
    m_model->reload();

    DynamicArray<int> duplicates = DataValidator::findDuplicateRows(data);
    QSet<int> duplicateSet;
    for (int i = 0; i < duplicates.size(); ++i)
        duplicateSet.insert(duplicates[i]);
    m_model->setDuplicateRows(duplicateSet);

    m_summary->setText(QString("<b>%1</b> rows x <b>%2</b> columns").arg(data.rowCount()).arg(data.columnCount()));

    fillColumnCombo(m_cleanColumn, false, "(all columns)");
    fillColumnCombo(m_editColumn, false);
    fillColumnCombo(m_searchColumn, false);
    m_index.clear();
    m_indexVersion = -1;
    m_searchResult->clear();
    updateIndexInfo();
}

MissingStrategy DatasetView::currentStrategy() const {
    switch (m_strategy->currentIndex()) {
    case 0: return MissingStrategy::Mean;
    case 1: return MissingStrategy::Median;
    case 2: return MissingStrategy::Zero;
    default: return MissingStrategy::RemoveRows;
    }
}

// ------------------------------- cleaning ----------------------------------

void DatasetView::removeDuplicates() {
    if (!m_workspace.hasData()) return;
    m_workspace.beginChange("Remove duplicate rows");
    int removed = DataCleaner::removeDuplicates(m_workspace.data());
    m_workspace.endChange("Removed " + std::to_string(removed) + " duplicate rows");
    emit statusMessage(QString("Removed %1 duplicate rows").arg(removed));
    emit dataModified();
}

void DatasetView::handleMissing() {
    if (!m_workspace.hasData()) return;
    int col = selectedColumn(m_cleanColumn);
    MissingStrategy strategy = currentStrategy();
    std::string target = col < 0 ? "all columns" : m_workspace.data().columnName(col);
    m_workspace.beginChange("Handle missing values (" + target + ")");
    int changed = DataCleaner::handleMissing(m_workspace.data(), col, strategy);
    std::string what = strategy == MissingStrategy::RemoveRows ? " rows removed" : " cells filled";
    m_workspace.endChange("Missing values in " + target + " -> " + DataCleaner::strategyName(strategy) +
                          " (" + std::to_string(changed) + what + ")");
    emit statusMessage(QString::number(changed) + Style::toQ(what));
    emit dataModified();
}

void DatasetView::fixInvalid() {
    if (!m_workspace.hasData()) return;
    int col = selectedColumn(m_cleanColumn);
    std::string target = col < 0 ? "all columns" : m_workspace.data().columnName(col);
    m_workspace.beginChange("Fix invalid values (" + target + ")");
    int changed = DataCleaner::fixInvalid(m_workspace.data(), col, currentStrategy());
    m_workspace.endChange("Fixed " + std::to_string(changed) + " invalid values in " + target + " (" +
                          DataCleaner::strategyName(currentStrategy()) + ")");
    emit statusMessage(QString("Fixed %1 invalid values").arg(changed));
    emit dataModified();
}

void DatasetView::trimWhitespace() {
    if (!m_workspace.hasData()) return;
    m_workspace.beginChange("Trim whitespace");
    int changed = DataCleaner::trimWhitespace(m_workspace.data());
    m_workspace.endChange("Trimmed whitespace in " + std::to_string(changed) + " cells");
    emit dataModified();
}

void DatasetView::renameColumn() {
    int col = selectedColumn(m_editColumn);
    QString name = m_newName->text().trimmed();
    if (col < 0 || name.isEmpty()) {
        QMessageBox::information(this, "Rename column", "Choose a column and type a new name.");
        return;
    }
    std::string oldName = m_workspace.data().columnName(col);
    m_workspace.beginChange("Rename column " + oldName);
    DataCleaner::renameColumn(m_workspace.data(), col, Style::toStd(name));
    m_workspace.endChange("Renamed column '" + oldName + "' to '" + Style::toStd(name) + "'");
    m_newName->clear();
    emit dataModified();
}

void DatasetView::removeColumn() {
    int col = selectedColumn(m_editColumn);
    if (col < 0) return;
    std::string name = m_workspace.data().columnName(col);
    if (QMessageBox::question(this, "Remove column", "Remove column '" + Style::toQ(name) + "'?\n(You can undo this.)")
        != QMessageBox::Yes)
        return;
    m_workspace.beginChange("Remove column " + name);
    DataCleaner::removeColumn(m_workspace.data(), col);
    m_workspace.endChange("Removed column '" + name + "'");
    emit dataModified();
}

// ---------------------------- search & index ------------------------------

// (Re)builds the hash table + BST for a column only when needed.
bool DatasetView::ensureIndex(int col) {
    if (col < 0) return false;
    if (m_index.column() != col || m_indexVersion != m_workspace.version()) {
        m_index.build(m_workspace.data(), col);
        m_indexVersion = m_workspace.version();
    }
    return true;
}

void DatasetView::updateIndexInfo() {
    if (!m_index.isBuilt()) {
        m_indexInfo->setText("Pick a column to build its index.");
        return;
    }
    const HashTable& h = m_index.hash();
    QString text = QString("Hash table: %1 keys in %2 buckets, load factor %3, %4 collisions, "
                           "longest chain %5, rehashed %6 times.")
                       .arg(h.size()).arg(h.capacity()).arg(h.loadFactor(), 0, 'f', 2)
                       .arg(h.collisions()).arg(h.longestChain()).arg(h.rehashCount());
    if (m_index.hasTree())
        text += QString("\nBST: %1 distinct keys, height %2.").arg(m_index.tree().size()).arg(m_index.tree().height());
    else
        text += "\nBST: text column (range search needs a numeric column).";
    m_indexInfo->setText(text);
}

void DatasetView::showMatches(const DynamicArray<int>& rows, const QString& how) {
    QVector<int> list;
    list.reserve(rows.size());
    for (int i = 0; i < rows.size(); ++i)
        list.append(rows[i]);
    std::sort(list.begin(), list.end());          // GUI only: keep table order
    m_model->setMatches(list, m_onlyMatches->isChecked());
    m_searchResult->setText(QString("%1 matching rows %2").arg(list.size()).arg(how));
    updateIndexInfo();
}

void DatasetView::searchExact() {
    int col = selectedColumn(m_searchColumn);
    if (!ensureIndex(col)) return;
    std::string value = Dataset::trim(Style::toStd(m_exactValue->text()));
    const DynamicArray<int>* rows = m_index.findExact(value);
    showMatches(rows ? *rows : DynamicArray<int>(), "(hash lookup, O(1) average)");
}

void DatasetView::searchRange() {
    int col = selectedColumn(m_searchColumn);
    if (!ensureIndex(col)) return;
    if (!m_index.hasTree()) {
        m_searchResult->setText("Range search needs a numeric column.");
        return;
    }
    DynamicArray<int> rows = m_index.findRange(m_rangeLow->value(), m_rangeHigh->value());
    showMatches(rows, "(BST range query)");
}

void DatasetView::clearSearch() {
    m_model->clearMatches();
    m_searchResult->clear();
}
