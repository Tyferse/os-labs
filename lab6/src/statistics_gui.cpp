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
#include <QtCharts/QAreaSeries>
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
      lower_series(new QLineSeries()),
      upper_series(new QLineSeries()),
      area_series(new QAreaSeries(upper_series, lower_series)),
      axisX(new QDateTimeAxis()),
      axisY(new QValueAxis()),
      timer(new QTimer(this)),
      network_manager(new QNetworkAccessManager(this))
{
    area_series->setColor(QColor(255, 165, 0, 100));

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
    chart->addSeries(area_series);
    chart->legend()->hide();

    axisX->setTitleText("Время");
    axisY->setTitleText("Температура (°C)");
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    area_series->attachAxis(axisX);
    area_series->attachAxis(axisY);

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

                // Обновление таблицы
                hour_table->setRowCount(1);
                double temp = 0;
                QString time_str;
                for (int i = 0; i < new_history.size(); ++i) {
                    // time_str = new_history[i].first;
                    temp += new_history[i].second;

                    // hour_table->setItem(i, 0, new QTableWidgetItem(time_str));
                    // hour_table->setItem(i, 1, new QTableWidgetItem(QString::number(temp, 'f', 2)));
                }

                hour_table->setItem(0, 0, new QTableWidgetItem(new_history[new_history.size()-1].first));
                hour_table->setItem(0, 1, new QTableWidgetItem(QString::number(temp / new_history.size(), 'f', 2)));

                // Обновление графика
                QList<QPointF> points;
                double minY = 60., maxY = -70.;
                for (int i = 0; i < temper_history.size(); ++i) {
                    QDateTime dt = QDateTime::fromString(temper_history[i].first, "yyyy-MM-dd hh:mm:ss");
                    if (!dt.isValid()) 
                        continue;

                    points.append(QPointF(dt.toMSecsSinceEpoch(), temper_history[i].second));
                    minY = qMin(minY, temper_history[i].second);
                    maxY = qMax(maxY, temper_history[i].second);
                }
                
                upper_series->replace(points);
                
                // Обновление оси X с временными метками
                if (!points.isEmpty()) {
                    axisX->setRange(
                        QDateTime::fromMSecsSinceEpoch(qint64(points.first().x())),
                        QDateTime::fromMSecsSinceEpoch(qint64(points.last().x()))
                    );
                }

                // Обновление оси Y
                double margin = (maxY - minY) * 0.1;
                if (margin == 0) 
                    margin = 0.5;

                axisY->setRange(minY - margin, maxY + margin);

                QList<QPointF> lower_points;
                for (const auto &p : points)
                    lower_points.append(QPointF(p.x(), 0)); 
                
                lower_series->replace(lower_points);
            });
        }
        history_reply->deleteLater();
    });
}
