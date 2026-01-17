#include "statistics_gui.hpp"
#include <QMetaObject>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTimer>
#include <QNetworkRequest>
#include <QUrl>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QScatterSeries>
#include <QtCharts/QDateTimeAxis>
#include <QThread>
#include <iostream>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      central_widget(new QWidget(this)),
      main_layout(new QVBoxLayout(central_widget)),
      top_layout(new QHBoxLayout()),
      left_panel(new QWidget()),
      left_layout(new QVBoxLayout(left_panel)),
      curr_temp_label(new QLabel("Загрузка...", this)),
      curr_date_label(new QLabel("", this)),
      hour_table(new QTableWidget(0, 2, this)),
      right_panel(new QWidget()),
      right_layout(new QVBoxLayout(right_panel)),
      series(new QLineSeries()),
      axisX(new QValueAxis()),
      axisY(new QValueAxis()),
      timer(new QTimer(this)),
      network_manager(new QNetworkAccessManager(this))
{
    // central_widget = new QWidget(this);
    // main_layout = new QVBoxLayout(central_widget);
    // top_layout = new QHBoxLayout();
    // left_panel = new QWidget();
    // left_layout = new QVBoxLayout(left_panel);
    // curr_temp_label = new QLabel("Загрузка...", this);
    // curr_date_label = new QLabel("", this);
    // hour_table = new QTableWidget(0, 2, this);
    // right_panel = new QWidget();
    // right_layout = new QVBoxLayout(right_panel);
    // series = new QLineSeries();
    // axisX = new QValueAxis();
    // axisY = new QValueAxis();
    // timer = new QTimer(this);
    // network_manager = new QNetworkAccessManager(this);

    setup_chart();
    setup_ui();
    setCentralWidget(central_widget);

    connect(timer, &QTimer::timeout, this, &MainWindow::update_data);
    timer->start(3000);
    update_data();
}

MainWindow::~MainWindow() {}

void MainWindow::setup_ui() {
    curr_temp_label->setStyleSheet("font-size: 30px; color: orange;");
    curr_date_label->setStyleSheet("font-style: italic; color: gray;");
    curr_date_label->setTextFormat(Qt::PlainText);

    hour_table->setHorizontalHeaderLabels({"Час", "Средняя темп."});
    hour_table->horizontalHeader()->setStretchLastSection(true);

    left_layout->addWidget(new QLabel("Текущая температура"));
    left_layout->addWidget(curr_temp_label);
    left_layout->addWidget(curr_date_label);
    left_layout->addSpacing(20);
    left_layout->addWidget(new QLabel("Средняя за час"));
    left_layout->addWidget(hour_table);
    left_panel->setFixedWidth(300);

    right_layout->addWidget(new QLabel("График температуры (последние 3 минуты)"), 0, Qt::AlignCenter);
    right_layout->addWidget(chart_view);

    top_layout->addWidget(left_panel);
    top_layout->addWidget(right_panel);
    main_layout->addLayout(top_layout);
}

void MainWindow::setup_chart() {
    chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();

    axisX->setTitleText("Время");
    axisY->setTitleText("Температура (°C)");
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::update_data() {
    // Запрос к серверу
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:8080/current"));
    request.setRawHeader("User-Agent", "QtTemperatureClient");

    QNetworkReply *reply = network_manager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            if (obj.contains("temperature")) {
                curr_temper = obj["temperature"].toDouble();
                curr_timestamp = obj["timestamp"].toString();

                // Обновление UI в основном потоке
                QMetaObject::invokeMethod(this, [this]() {
                    curr_temp_label->setText(QString("%1 °C").arg(curr_temper, 0, 'f', 2));
                    curr_date_label->setText(QString("Данные на %1").arg(curr_timestamp));
                });
            }
        }
        reply->deleteLater();
    });

    // Запрос истории
    QNetworkRequest history_req;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime start = now.addSecs(-3 * 60); // последние 3 минуты
    QString start_str = start.toString("yyyy-MM-dd hh:mm:ss");
    QString end_str = now.toString("yyyy-MM-dd hh:mm:ss");
    QString url = QString("http://localhost:8080/history?start=%1&end=%2").arg(start_str).arg(end_str);
    history_req.setUrl(QUrl(url));
    history_req.setRawHeader("User-Agent", "QtTemperatureClient");

    QNetworkReply *history_reply = network_manager->get(history_req);
    connect(history_reply, &QNetworkReply::finished, [this, history_reply]() {
        if (history_reply->error() == QNetworkReply::NoError) {
            QByteArray data = history_reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonArray arr = doc.array();

            QList<QPair<QString, double>> new_history;
            for (const auto &value : arr) {
                QJsonObject obj = value.toObject();
                QString ts = obj["timestamp"].toString();
                double temp = obj["temperature"].toDouble();
                new_history.append(qMakePair(ts, temp));
            }

            // Обновление истории
            QMetaObject::invokeMethod(this, [this, new_history]() {
                temper_history = new_history;
                // Обновление графика
                QList<QPointF> points;
                for (int i = 0; i < temper_history.size(); ++i)
                    points.append(QPointF(i, temper_history[i].second));
                
                series->replace(points);

                // Обновление соси X с временными метками
                if (!points.isEmpty()) 
                    axisX->setRange(0, points.size() - 1);
            });
        }
        history_reply->deleteLater();
    });
}


// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);

//     MainWindow window;
//     window.setWindowTitle("Мониторинг температуры");
//     window.resize(1200, 800);
//     window.show();

//     return app.exec();
// }
