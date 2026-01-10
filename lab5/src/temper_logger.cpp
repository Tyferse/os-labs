#include "temper_db_logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>
#include <regex>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#define DB_PATH std::string("logs/temper_log.db")
#define FULL_LOG std::string("full_temperature")
#define HOUR_LOG std::string("hour_temperature")
#define DAY_LOG std::string("day_temperature")


TemperLogger::TemperLogger(const std::string& port_name)
    : port_name_(port_name) {
    cplib::SerialPort::Parameters params(cplib::SerialPort::BAUDRATE_115200);
    port_.Open(port_name_, params);
    port_.SetTimeout(1.0);

    if (!directory_exists("logs/"))
        create_single_directory("logs/");
    
    dbm_ = std::make_unique<DBManager>(DB_PATH);
    std::string table_names[3] = {FULL_LOG, HOUR_LOG, DAY_LOG};
    if (!dbm_->initialize(table_names, 3)) {
        std::cerr << "Failed to initialize database" << std::endl;
    }

    tcp_server_ = std::make_unique<TCPServer>(8080, [this](const std::string& request) {
        return this->handle_tcp_request(request);
    });
}

TemperLogger::~TemperLogger() {
    stop();
}

bool TemperLogger::start() {
    if (!port_.IsOpen()) {
        std::cerr << "Failed to open COM port: " << port_name_ << std::endl;
        return false;
    }

    running_ = true;
    read_thread_ = std::thread(&TemperLogger::read_loop, this);
    hour_thread_ = std::thread(&TemperLogger::avg_per_hour, this);
    day_thread_ = std::thread(&TemperLogger::avg_per_day, this);
    tcp_server_->start();
    return true;
}

void TemperLogger::stop() {
    running_ = false;
    if (read_thread_.joinable()) 
        read_thread_.join();
    if (hour_thread_.joinable()) 
        hour_thread_.join();
    if (day_thread_.joinable()) 
        day_thread_.join();

    tcp_server_->stop();
}

void TemperLogger::read_loop() {
    std::string line;
    while (running_) {
        port_>> line;
        if (!line.empty()) {
            std::istringstream iss(line);
            double temper1, temper2;
            if (iss >> temper1 >> temper2) 
                if (temper1 == temper2)
                    process_temper(temper1);
        }
    }
}

void TemperLogger::process_temper(double temper) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!dbm_->insert_temper(FULL_LOG, get_curr_time(), temper))
        std::cerr << "Failed to insert temperature into database" << std::endl;
}

void TemperLogger::avg_per_hour() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::hours(1));

        mutex_.lock();

        std::string now = get_curr_time();
        auto records = dbm_->get_temper_for_period(
            FULL_LOG, get_time_offset(-3600), now
        );
        if (!records.empty()) {
            double sum = 0.0;
            for (const auto& r : records)
                sum += r.temperature;
            
            double avg = sum / records.size();
            dbm_->insert_temper(HOUR_LOG, now, avg);
        }

        mutex_.unlock();
    }
}

void TemperLogger::avg_per_day() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::hours(24));

        mutex_.lock();
        
        std::string now = get_curr_time();
        auto records = dbm_->get_temper_for_period(
            FULL_LOG, get_time_offset(-86400), now
        );
        if (!records.empty()) {
            double sum = 0.0;
            for (const auto& r : records)
                sum += r.temperature;
            
            double avg = sum / records.size();
            dbm_->insert_temper(DAY_LOG, now, avg);
        }

        // Очистка логов
        dbm_->clear_logs(FULL_LOG, get_time_offset(-86400));
        dbm_->clear_logs(HOUR_LOG, get_time_offset(-30*86400));
        dbm_->clear_logs(DAY_LOG, get_time_offset(-365*86400));
        
        mutex_.unlock();
    }
}

std::string TemperLogger::handle_tcp_request(const std::string& request) {
    size_t end_first_line = request.find("\r\n");
    if (end_first_line == std::string::npos) {
        return "{\"error\": \"Bad request\"}";
    }

    std::string first_line = request.substr(0, end_first_line);
    size_t start = first_line.find(' ');
    size_t end = first_line.find(' ', start + 1);
    if (start == std::string::npos || end == std::string::npos) {
        return "{\"error\": \"Bad request\"}";
    }

    std::string url = first_line.substr(start + 1, end - start - 1);

    if (url == "/current") {
        auto all_records = dbm_->get_last_temper(FULL_LOG);
        if (!all_records.empty()) {
            std::string last_data = all_records.back().timestamp;
            double last_temp = all_records.back().temperature;
            std::ostringstream oss;
            oss << "{\"timestamp\": \"" << last_data << "\", \"temperature\": " << last_temp << "}";
            return oss.str();
        } 
        else
            return "{\"error\": \"No temperature data available\"}";
    }
    else if (url.substr(0, 8) == "/history") {
        std::string query_str = url.substr(url.find('?') + 1);
        std::string start_time = "1970-01-01 00:00:00";
        std::string end_time = get_curr_time();

        if (!query_str.empty()) {
            std::regex pattern(R"(start=([^&]+)&end=([^&]+))");
            std::smatch matches;
            if (std::regex_search(query_str, matches, pattern)) {
                start_time = url_decode(matches[1].str());
                end_time = url_decode(matches[2].str());
            }
        }

        auto records = dbm_->get_temper_for_period(FULL_LOG, start_time, end_time);
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < records.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "{\"timestamp\": \"" << records[i].timestamp << "\", \"temperature\": " << records[i].temperature << "}";
        }
        oss << "]";
        return oss.str();
    }
    else
        return "{\"error\": \"Invalid endpoint. Use /current or /history?start=...&end=...\"}";
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: temper_logger [port]" << std::endl;
        return 1;
    }

    TemperLogger logger(argv[1]);
    if (!logger.start()) {
        std::cerr << "Failed to start logger" << std::endl;
        return 1;
    }

    std::cout << "Logger started on port: " << argv[1] << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    std::cin.get();

    return 0;
}
