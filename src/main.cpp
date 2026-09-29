#include "UI/MainWindow.h"
#include "UI/Style.h"
#include <QApplication>
#include <QFont>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("PRISM");
    app.setStyle("Fusion");                    // same look on every PC
    app.setStyleSheet(Style::appStyleSheet());
    QFont font("Segoe UI", 10);
    app.setFont(font);

    MainWindow window;
    window.show();
    return app.exec();
}
