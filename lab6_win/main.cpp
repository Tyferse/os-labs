#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[]) {
    qDebug() << "Plugin paths:" << QCoreApplication::libraryPaths();

    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Qt 6.10 Test");

    QLabel label("Qt 6.10 работает!");
    QPushButton button("Нажми меня");

    QVBoxLayout layout(&window);
    layout.addWidget(&label);
    layout.addWidget(&button);

    QObject::connect(&button, &QPushButton::clicked, []() {
        qDebug("Кнопка нажата!");
    });

    window.show();
    return app.exec();
}
