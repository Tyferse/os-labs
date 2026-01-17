#include <QApplication>
#include <iostream>
#include "statistics_gui.hpp"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;

    window.setWindowTitle("Мониторинг температуры");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
