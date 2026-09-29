#ifndef PRISM_DATASET_TABLE_MODEL_H
#define PRISM_DATASET_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QSet>
#include <QVector>

class Dataset;

// ---------------------------------------------------------------------------
// DatasetTableModel - connects our Dataset to Qt's QTableView.
//
// Qt asks the model only for the cells that are visible on screen, so even
// tens of thousands of rows scroll smoothly.
// Colours: missing = red, invalid = orange, duplicate row = purple,
//          search match = yellow.
// ---------------------------------------------------------------------------
class DatasetTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit DatasetTableModel(const Dataset& data, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void reload();                                     // dataset changed completely
    void setDuplicateRows(const QSet<int>& rows);
    void setMatches(const QVector<int>& rows, bool showOnlyMatches);
    void clearMatches();

    int datasetRow(int viewRow) const;                 // view row -> dataset row

private:
    const Dataset& m_data;
    QSet<int> m_duplicates;
    QSet<int> m_matchSet;
    QVector<int> m_visibleRows;                        // used when filtering
    bool m_filtered = false;
};

#endif // PRISM_DATASET_TABLE_MODEL_H
