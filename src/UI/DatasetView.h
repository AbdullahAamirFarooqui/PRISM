#ifndef PRISM_DATASET_VIEW_H
#define PRISM_DATASET_VIEW_H

#include "UI/BaseView.h"
#include "Data/ColumnIndex.h"
#include "Data/DataCleaner.h"

class QTableView;
class QComboBox;
class QLineEdit;
class QDoubleSpinBox;
class QCheckBox;
class QLabel;
class DatasetTableModel;

// ---------------------------------------------------------------------------
// DatasetView - the spreadsheet-like screen.
//   left : the data table (missing / invalid / duplicate cells are coloured)
//   right: cleaning tools, column tools and "Search & Index"
//          (exact search with the HashTable, range search with the BST)
// Every cleaning action is undoable (Workspace::beginChange).
// ---------------------------------------------------------------------------
class DatasetView : public BaseView {
    Q_OBJECT
public:
    explicit DatasetView(Workspace& workspace, QWidget* parent = nullptr);
    QString viewName() const override { return "Dataset"; }

protected:
    void refresh() override;

private slots:
    void removeDuplicates();
    void handleMissing();
    void fixInvalid();
    void trimWhitespace();
    void renameColumn();
    void removeColumn();
    void searchExact();
    void searchRange();
    void clearSearch();

private:
    QWidget* buildToolPanel();
    MissingStrategy currentStrategy() const;
    bool ensureIndex(int col);
    void showMatches(const DynamicArray<int>& rows, const QString& how);
    void updateIndexInfo();

    DatasetTableModel* m_model;
    QTableView* m_table;
    QLabel* m_summary;

    QComboBox* m_cleanColumn;
    QComboBox* m_strategy;
    QComboBox* m_editColumn;
    QLineEdit* m_newName;

    QComboBox* m_searchColumn;
    QLineEdit* m_exactValue;
    QDoubleSpinBox* m_rangeLow;
    QDoubleSpinBox* m_rangeHigh;
    QCheckBox* m_onlyMatches;
    QLabel* m_searchResult;
    QLabel* m_indexInfo;

    ColumnIndex m_index;
    int m_indexVersion = -1;
};

#endif // PRISM_DATASET_VIEW_H
