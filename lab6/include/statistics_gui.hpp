#pragma once

#include <QMainWindow>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


QT_BEGIN_NAMESPACE
class QLabel;
class QTableWidget;
class QTimer;
class QNetworkAccessManager;
class QChart;
class QChartView;
class QLineSeries;
class QValueAxis;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    // Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

// private slots:
    // void update_data();

private:
    void setup_ui();
    void setup_chart();
    void update_data();

    QWidget *central_widget;
    QVBoxLayout *main_layout;
    QHBoxLayout *top_layout;

    QWidget *left_panel;
    QVBoxLayout *left_layout;
    QLabel *curr_temp_label;
    QLabel *curr_date_label;
    QTableWidget *hour_table;

    QWidget *right_panel;
    QVBoxLayout *right_layout;
    QChart *chart;
    QChartView *chart_view;
    QLineSeries *series;
    QValueAxis *axisX;
    QValueAxis *axisY;

    QTimer *timer;
    QNetworkAccessManager *network_manager;

    // Данные
    QString curr_timestamp;
    double curr_temper = 0.0;
    QList<QPair<QString, double>> temper_history;
    QMap<QString, double> hour_avg_today;
};
