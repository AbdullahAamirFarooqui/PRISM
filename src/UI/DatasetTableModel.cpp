#include "UI/DatasetTableModel.h"
#include "UI/Style.h"
#include "Data/Dataset.h"
#include <QBrush>

DatasetTableModel::DatasetTableModel(const Dataset& data, QObject* parent)
    : QAbstractTableModel(parent), m_data(data) {}

int DatasetTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return m_filtered ? m_visibleRows.size() : m_data.rowCount();
}

int DatasetTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return m_data.columnCount();
}

int DatasetTableModel::datasetRow(int viewRow) const {
    return m_filtered ? m_visibleRows[viewRow] : viewRow;
}

QVariant DatasetTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();
    int row = datasetRow(index.row());
    int col = index.column();

    if (role == Qt::DisplayRole)
        return Style::toQ(m_data.cell(row, col));

    if (role == Qt::BackgroundRole) {
        if (m_data.isMissing(row, col)) return QBrush(Style::MissingCell);
        if (m_data.isInvalid(row, col)) return QBrush(Style::InvalidCell);
        if (m_matchSet.contains(row)) return QBrush(Style::MatchRow);
        if (m_duplicates.contains(row)) return QBrush(Style::DuplicateRow);
    }
    if (role == Qt::ToolTipRole) {
        if (m_data.isMissing(row, col)) return "Missing value";
        if (m_data.isInvalid(row, col)) return "Invalid value (not a number)";
        if (m_duplicates.contains(row)) return "Duplicate of an earlier row";
    }
    if (role == Qt::TextAlignmentRole && m_data.isNumeric(col))
        return int(Qt::AlignRight | Qt::AlignVCenter);
    return QVariant();
}

QVariant DatasetTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section >= m_data.columnCount())
            return QVariant();
        QString type = m_data.isNumeric(section) ? "123" : "abc";
        return Style::toQ(m_data.columnName(section)) + "  [" + type + "]";
    }
    return datasetRow(section) + 1;            // rows are numbered from 1 for users
}

void DatasetTableModel::reload() {
    beginResetModel();
    m_matchSet.clear();
    m_visibleRows.clear();
    m_filtered = false;
    endResetModel();
}

void DatasetTableModel::setDuplicateRows(const QSet<int>& rows) {
    beginResetModel();
    m_duplicates = rows;
    endResetModel();
}

void DatasetTableModel::setMatches(const QVector<int>& rows, bool showOnlyMatches) {
    beginResetModel();
    m_matchSet = QSet<int>(rows.begin(), rows.end());
    m_visibleRows = rows;
    m_filtered = showOnlyMatches;
    endResetModel();
}

void DatasetTableModel::clearMatches() {
    beginResetModel();
    m_matchSet.clear();
    m_visibleRows.clear();
    m_filtered = false;
    endResetModel();
}
