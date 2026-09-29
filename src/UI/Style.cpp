#include "UI/Style.h"
#include "Data/Dataset.h"
#include <QLabel>
#include <QDir>
#include <cmath>

namespace Style {

QString appStyleSheet() {
    return R"(
        QMainWindow, QWidget#page { background: #f7f7fb; }
        QTabWidget::pane { border: 1px solid #dcdbe8; background: #ffffff; border-radius: 6px; }
        QTabBar::tab { background: #eceaf6; color: #333; padding: 8px 18px; margin-right: 2px;
                       border-top-left-radius: 6px; border-top-right-radius: 6px; }
        QTabBar::tab:selected { background: #5b4fcf; color: white; }
        QTabBar::tab:hover:!selected { background: #dcd8f5; }
        QGroupBox { border: 1px solid #dcdbe8; border-radius: 6px; margin-top: 14px; padding: 10px 8px 8px 8px;
                    background: #ffffff; font-weight: bold; color: #3b2c85; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
        QGroupBox QLabel, QGroupBox QCheckBox, QGroupBox QRadioButton { font-weight: normal; color: #222; }
        QPushButton { background: #5b4fcf; color: white; border: none; padding: 6px 14px; border-radius: 5px; }
        QPushButton:hover { background: #4a3fb8; }
        QPushButton:pressed { background: #3b2c85; }
        QPushButton:disabled { background: #c8c6d8; color: #f2f2f2; }
        QPushButton[secondary="true"] { background: #eceaf6; color: #3b2c85; }
        QPushButton[secondary="true"]:hover { background: #dcd8f5; }
        QComboBox, QLineEdit, QSpinBox, QDoubleSpinBox { padding: 4px 6px; border: 1px solid #cfcde0;
                    border-radius: 4px; background: white; min-height: 20px; }
        QTableView, QTableWidget, QListWidget, QPlainTextEdit, QTextBrowser { border: 1px solid #dcdbe8;
                    background: white; gridline-color: #ecebf3; selection-background-color: #5b4fcf; }
        QHeaderView::section { background: #f0eefa; color: #3b2c85; padding: 5px; border: none;
                    border-right: 1px solid #dcdbe8; border-bottom: 1px solid #dcdbe8; font-weight: bold; }
        QProgressBar { border: 1px solid #cfcde0; border-radius: 4px; text-align: center; background: white; }
        QProgressBar::chunk { background: #5b4fcf; border-radius: 3px; }
        QDockWidget::title { background: #eceaf6; padding: 6px; color: #3b2c85; font-weight: bold; }
        QFrame#card { background: white; border: 1px solid #dcdbe8; border-radius: 8px; }
        QToolBar { background: #ffffff; border-bottom: 1px solid #dcdbe8; spacing: 4px; padding: 3px; }
        QToolBar QToolButton { padding: 5px 10px; border-radius: 4px; color: #3b2c85; }
        QToolBar QToolButton:hover { background: #eceaf6; }
        QToolBar QToolButton:disabled { color: #aaa; }
    )";
}

QLabel* makeTitle(const QString& text, QWidget* parent) {
    QLabel* label = new QLabel(text, parent);
    label->setStyleSheet("font-size: 20px; font-weight: bold; color: #3b2c85; padding: 4px 0;");
    return label;
}

QLabel* makeHint(const QString& text, QWidget* parent) {
    QLabel* label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setStyleSheet("color: #666; font-size: 12px;");
    return label;
}

QColor correlationColor(double r) {
    if (std::isnan(r))
        return QColor("#eeeeee");
    int intensity = static_cast<int>(std::fabs(r) * 170);
    if (r >= 0)
        return QColor(255 - intensity, 255 - intensity / 2, 255);
    return QColor(255, 255 - intensity, 255 - intensity);
}

std::string toStdPath(const QString& path) {
    return QDir::toNativeSeparators(path).toLocal8Bit().toStdString();
}

QString formatNumber(double value, int decimals) {
    return QString::fromStdString(Dataset::formatNumber(value, decimals));
}

} // namespace Style
