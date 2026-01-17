#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    QWidget centralWidget;
    QVBoxLayout *layout = new QVBoxLayout(&centralWidget);
    QLineSeries *series = new QLineSeries();
    
    // Заполняем данные
    for (int i = 0; i <= 100; ++i) {
        double x = i / 10.0;
        double y = qSin(x);  
        series->append(x, y);
    }

    // Создаем график
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("График функции sin(x)");
    chart->createDefaultAxes(); 
    
    // Настройка подписей осей
    chart->axes(Qt::Horizontal).first()->setTitleText("x");
    chart->axes(Qt::Vertical).first()->setTitleText("sin(x)");

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    layout->addWidget(chartView);

    window.setCentralWidget(&centralWidget);
    window.resize(800, 600);
    window.show();

    return app.exec();
}
