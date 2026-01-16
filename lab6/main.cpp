#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include "qcustomplot.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    QWidget centralWidget;
    QVBoxLayout *layout = new QVBoxLayout(&centralWidget);

    QCustomPlot *customPlot = new QCustomPlot(&centralWidget);
    layout->addWidget(customPlot);

    // Добавляем график
    customPlot->addGraph();
    QVector<double> x(101), y(101); // 101 точка

    for (int i = 0; i < 101; ++i) {
        x[i] = i / 10.0; // x от 0 до 10
        y[i] = qSin(x[i]); // sin(x)
    }

    customPlot->graph(0)->setData(x, y);
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("sin(x)");
    customPlot->replot();

    window.setCentralWidget(&centralWidget);
    window.resize(800, 600);
    window.show();

    return app.exec();
}
