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
      curr_hour_key(QDateTime::fromString(
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:00:00"), 
        "yyyy-MM-dd hh:00:00")
      ),
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
    setStyleSheet(R"(
        font-family: Arial, sans-serif;
        background-color: rgba(45, 36, 24, 0.99);
        color: white;
        padding: 20px;
    )");

    setup_chart();
    setup_ui();
    find_initial_history();
    setCentralWidget(central_widget);

    connect(timer, &QTimer::timeout, this, &MainWindow::update_data);
    timer->start(3000);
    update_data();
}

MainWindow::~MainWindow() {}

void MainWindow::setup_ui() {
    curr_temp_label->setStyleSheet("font-size: 4em; color: #ff6000; font-weight: bold; padding-bottom: 1em;");
    curr_date_label->setStyleSheet("font-style: italic; color: lightgray;");
    curr_date_label->setTextFormat(Qt::PlainText);

    // hour_table->setHorizontalHeaderLabels({"Час", "Средняя темп."});
    // hour_table->horizontalHeader()->setStretchLastSection(true);
    QHeaderView *header = hour_table->horizontalHeader();
    header->setStyleSheet(R"(
        QHeaderView::section {
            background-color: #d4a65c;
            color: #44ade1;
            font-size: 14pt;
            font-weight: bold;
        }
    )");
    hour_table->setHorizontalHeaderLabels({"Час", "Средняя темп."});
    // hour_table->horizontalHeader()->setStretchLastSection(true);

    hour_table->setStyleSheet("background-color: #887878;");
    hour_table->viewport()->setStyleSheet("background-color: #665959;");
    // hour_table->setStyleSheet(R"(
    //     QTableWidget {
    //         gridline-color: #d0d0d0;
    //         font-size: 12pt;
    //         color: #d0d0d0;
    //     }
    //     QTableWidget::item:selected {
    //         background-color: #c7a16c;
    //     }
    // )");
    // hour_table->viewport()->setStyleSheet("background-color: #9b7b50;");

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

    // Фон графика
    chart->setBackgroundBrush(QColor("#986a43"));
    // chart->setPlotAreaBackgroundBrush(QColor("#2e2e2e"));
    // chart->setPlotAreaBackgroundVisible(true);

    // Оси
    axisX->setTitleText("Время");
    axisY->setTitleText("Температура (°C)");

    // Цвет текста подписей осей
    QFont axisFont;
    axisFont.setPointSize(10);
    axisX->setLabelsFont(axisFont);
    axisY->setLabelsFont(axisFont);
    axisX->setLabelsColor(QColor("white"));
    axisY->setLabelsColor(QColor("white"));

    // Цвет заголовков осей
    axisX->setTitleBrush(QBrush(QColor("white")));
    axisY->setTitleBrush(QBrush(QColor("white")));
    axisX->setTitleFont(QFont("Arial", 12, QFont::Bold));
    axisY->setTitleFont(QFont("Arial", 12, QFont::Bold));

    // Сетка
    axisX->setGridLineVisible(true);
    axisY->setGridLineVisible(true);
    axisX->setGridLineColor(QColor("rgba(255, 255, 255, 0.7)"));
    axisY->setGridLineColor(QColor("rgba(255, 255, 255, 0.7)"));

    // Линии осей (основные)
    axisX->setLinePenColor(QColor("#cccccc"));
    axisY->setLinePenColor(QColor("#cccccc"));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    area_series->attachAxis(axisX);
    area_series->attachAxis(axisY);

    // Цвет самой линии и заливки
    upper_series->setColor(QColor("rgba(255, 98, 0, 0.9)")); 
    area_series->setBorderColor(QColor("rgba(255, 96, 0, 0.9)"));
    area_series->setColor(QColor("rgb(255, 98, 0)"));

    chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    // chart_view->setStyleSheet("background-color: #2e2e2e;");
}

void MainWindow::find_initial_history() {
    QNetworkRequest history_req;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime start = now.addSecs(-12 * 3600); // данные за 12 часов
    QString start_str = start.toString("yyyy-MM-dd hh:00:00");
    QString end_str = now.toString("yyyy-MM-dd hh:mm:ss");
    QString url = QString("http://localhost:8080/history?start=%1&end=%2").arg(start_str).arg(end_str);
    history_req.setUrl(QUrl(url));
    history_req.setRawHeader("User-Agent", "QtTemperatureClient");

    QNetworkReply *history_reply = network_manager->get(history_req);
    connect(history_reply, &QNetworkReply::finished, [this, history_reply, start]() {
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
            QMetaObject::invokeMethod(this, [this, new_history, start]() {
                temper_history = new_history;
                QDateTime history_hour = start;
                double avg;

                // Добавляем точки из истории за последние 12 часов, если они есть
                for (const auto &point : new_history) {
                    QDateTime dt = QDateTime::fromString(point.first, "yyyy-MM-dd hh:mm:ss");

                    if (history_hour <= dt && dt < history_hour.addSecs(3600)) {
                        curr_hour_temper.append(point.second);
                    }
                    else {
                        avg = 0.0;
                        for (int i = 0; i < curr_hour_temper.size(); i++)
                            avg += curr_hour_temper[i];

                        avg /= curr_hour_temper.size();
                        hour_avg_today.append(qMakePair(history_hour, avg));

                        while (history_hour > dt || dt >= history_hour.addSecs(3600))
                            history_hour = history_hour.addSecs(3600);
                    
                        curr_hour_temper.clear();
                    }
                }

                update_hour_table();
            });
        }
        history_reply->deleteLater();
    });
}

void MainWindow::update_hour_table() {
    // Считаем среднее за текущий час
    double curr_avg = 0.0;
    if (!curr_hour_temper.isEmpty()) {
        for (int i = 0; i < curr_hour_temper.size(); i++)
            curr_avg += curr_hour_temper[i];

        curr_avg /= curr_hour_temper.size();
    }

    int total_rows = qMin(12, 1 + static_cast<int>(hour_avg_today.size()));
    hour_table->setRowCount(total_rows);

    // Первая строка — текущий час
    hour_table->setItem(0, 0, new QTableWidgetItem(curr_hour_key.toString("dd.MM.yyyy hh:mm")));
    // Добавляем стиль к ячейкам с температурой
    auto *item = new QTableWidgetItem(QString::number(curr_avg, 'f', 2));
    item->setForeground(QColor("#ff6000"));
    item->setTextAlignment(Qt::AlignCenter);
    item->setFont(QFont("Arial", 13));
    hour_table->setItem(0, 1, item);

    // Остальные строки — прошлые часы
    for (int i = 0; i < total_rows - 1; ++i) {
        const auto &entry = hour_avg_today[hour_avg_today.size() - 1 - i];
        hour_table->setItem(i + 1, 0, new QTableWidgetItem(entry.first.toString("dd.MM.yyyy hh:mm")));
        hour_table->setItem(i + 1, 1, new QTableWidgetItem(QString::number(entry.second, 'f', 2)));
    }
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

                // Получаем текущее время и начало текущего часа
                QDateTime new_hour_key = QDateTime::fromString(
                    QDateTime::currentDateTime().toString("yyyy-MM-dd hh:00:00"), 
                    "yyyy-MM-dd hh:00:00"
                );

                // Проверяем, сменился ли час
                if (new_hour_key != curr_hour_key) {
                    if (!curr_hour_temper.isEmpty()) {
                        double avg = 0.0; 
                        for (int i = 0; i < curr_hour_temper.size(); i++)
                            avg += curr_hour_temper[i];

                        avg /= curr_hour_temper.size();
                        hour_avg_today.append(qMakePair(curr_hour_key, avg));

                        if (hour_avg_today.size() > 11)
                            hour_avg_today.removeFirst();
                    }

                    curr_hour_key = new_hour_key;
                    curr_hour_temper.clear();
                }

                // Добавляем новые точки из истории в текущий час
                for (const auto &point : new_history) {
                    QDateTime dt = QDateTime::fromString(point.first, "yyyy-MM-dd hh:mm:ss");
                    if (curr_hour_key <= dt && dt < curr_hour_key.addSecs(3600)) {
                        curr_hour_temper.append(point.second);
                    }
                }

                // Обновляем таблицу
                update_hour_table();

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
