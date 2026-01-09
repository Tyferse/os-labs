#include "db.hpp"
#include <iostream>
#include <sstream>


DBManager::DBManager(const std::string& db_path) {
    sqlite3* temp_db = nullptr;
    int rc = sqlite3_open(db_path.c_str(), &temp_db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(temp_db) << std::endl;
        sqlite3_close(temp_db);
    } 
    else
        db_.reset(temp_db);
}

DBManager::~DBManager() {}

bool DBManager::initialize(std::string table_names[], int n_tables) {
    if (!db_) 
        return false;

    std::string sql;
    int rc;
    char* err_msg;
    for (int i = 0; i < n_tables; i++) {
        sql = "CREATE TABLE IF NOT EXISTS " + table_names[i] + " " + R"(
            (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                temperature REAL NOT NULL
            )
        )";
        err_msg = nullptr;
        rc = sqlite3_exec(db_.get(), sql.c_str(), nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << err_msg << std::endl;
            sqlite3_free(err_msg);
            return false;
        }
    }
    return true;
}

bool DBManager::insert_temper(const std::string& table_name, const std::string& timestamp, double temper) {
    if (!db_) 
        return false;

    std::lock_guard<std::mutex> lock(db_mutex_);

    std::string sql = "INSERT INTO " + table_name + " (timestamp, temperature) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Insert prepare error: " << sqlite3_errmsg(db_.get()) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, temper);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insert step error: " << sqlite3_errmsg(db_.get()) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::vector<TempeRecord> DBManager::get_temper_for_period(
    const std::string& table_name, const std::string& start_time, const std::string& end_time
) {
    if (!db_) 
        return {};

    std::lock_guard<std::mutex> lock(db_mutex_);

    std::string sql = "SELECT timestamp, temperature FROM " + table_name 
                      + " WHERE timestamp BETWEEN ? AND ? ORDER BY timestamp;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Query prepare error: " << sqlite3_errmsg(db_.get()) << std::endl;
        return {};
    }

    sqlite3_bind_text(stmt, 1, start_time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_time.c_str(), -1, SQLITE_STATIC);

    std::vector<TempeRecord> records;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* ts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double temper = sqlite3_column_double(stmt, 1);
        records.push_back({std::string(ts), temper});
    }

    sqlite3_finalize(stmt);
    return records;
}

std::vector<TempeRecord> DBManager::get_last_temper(const std::string& table_name, int n_last_records) {
    if (!db_) 
        return {};

    std::lock_guard<std::mutex> lock(db_mutex_);

    std::string sql = "SELECT timestamp, temperature FROM " + table_name + " ORDER BY timestamp DESC LIMIT ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Query prepare error: " << sqlite3_errmsg(db_.get()) << std::endl;
        return {};
    }

    sqlite3_bind_int(stmt, 1, n_last_records);

    std::vector<TempeRecord> records;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* ts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double temper = sqlite3_column_double(stmt, 1);
        records.push_back({std::string(ts), temper});
    }

    sqlite3_finalize(stmt);
    return records;
}

void DBManager::clear_logs(const std::string& table_name, const std::string& threshold_time) {
    if (!db_) 
        return;

    std::string sql = "DELETE FROM " + table_name + " WHERE timestamp < ?;";
    int rc;
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Delete prepare error: " << sqlite3_errmsg(db_.get()) << std::endl;
    }

    sqlite3_bind_text(stmt, 1, threshold_time.c_str(), -1, SQLITE_STATIC);

    char* err_msg = nullptr;
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
        std::cerr << "Deletion error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
}
