#ifndef PRISM_STYLE_H
#define PRISM_STYLE_H

#include <QString>
#include <QColor>
#include <string>

class QLabel;
class QWidget;

// ---------------------------------------------------------------------------
// Small GUI helpers shared by all screens: colours, the application style
// sheet and std::string <-> QString conversion.
// ---------------------------------------------------------------------------
namespace Style {

// Colour palette
const QColor Accent("#5b4fcf");
const QColor AccentLight("#ecebfb");
const QColor Positive("#2f6fdf");
const QColor Negative("#d9480f");
const QColor MissingCell("#ffd6d6");
const QColor InvalidCell("#ffe2b8");
const QColor DuplicateRow("#e6dcff");
const QColor MatchRow("#fff3a3");

QString appStyleSheet();

// Big coloured heading label used at the top of each screen.
QLabel* makeTitle(const QString& text, QWidget* parent = nullptr);
QLabel* makeHint(const QString& text, QWidget* parent = nullptr);

// Colour for a correlation coefficient (blue = positive, red = negative).
QColor correlationColor(double r);

inline QString toQ(const std::string& s) { return QString::fromStdString(s); }
inline std::string toStd(const QString& s) { return s.toStdString(); }
// File paths: use the local 8-bit encoding so std::ifstream can open them on Windows.
std::string toStdPath(const QString& path);

QString formatNumber(double value, int decimals = 4);

} // namespace Style

#endif // PRISM_STYLE_H
