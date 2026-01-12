#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <mutex>
#include <memory>

struct TempeRecord {
    std::string timestamp;
    double temperature;
};

class DBManager {
public:
    explicit DBManager(const std::string& db_path);
    ~DBManager();

    bool initialize(std::string table_names[], int n_tables);
    bool insert_temper(const std::string& table_name, const std::string& timestamp, double temp);
    std::vector<TempeRecord> get_temper_for_period(const std::string& table_name, const std::string& start_time, const std::string& end_time);
    std::vector<TempeRecord> get_last_temper(const std::string& table_name, int n_last_records=1);
    void clear_logs(const std::string& table_name, const std::string& threshold_time);

private:
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db_{nullptr, sqlite3_close};
    std::mutex db_mutex_;
};
