#include "temper_db_logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>

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
}

TemperLogger::~TemperLogger() {
    stop();
}

bool TemperLogger::start() {
    if (!port_.IsOpen()) {
        std::cerr << "Failed to open port: " << port_name_ << std::endl;
        return false;
    }

    running_ = true;
    read_thread_ = std::thread(&TemperLogger::read_loop, this);
    hour_thread_ = std::thread(&TemperLogger::avg_per_hour, this);
    day_thread_ = std::thread(&TemperLogger::avg_per_day, this);
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
        std::this_thread::sleep_for(std::chrono::seconds(30));

        mutex_.lock();

        std::string now = get_curr_time();
        auto records = dbm_->get_temper_for_period(
            FULL_LOG, get_time_offset(-30), now //-3600), now
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
        std::this_thread::sleep_for(std::chrono::minutes(1));

        mutex_.lock();
        
        std::string now = get_curr_time();
        auto records = dbm_->get_temper_for_period(
            FULL_LOG, get_time_offset(-60), now //-86400), now
        );
        if (!records.empty()) {
            double sum = 0.0;
            for (const auto& r : records)
                sum += r.temperature;
            
            double avg = sum / records.size();
            dbm_->insert_temper(DAY_LOG, now, avg);
        }

        // Очистка логов
        dbm_->clear_logs(FULL_LOG, get_time_offset(-60)); //-86400));
        dbm_->clear_logs(HOUR_LOG, get_time_offset(-120)); //-30*86400));
        dbm_->clear_logs(DAY_LOG, get_time_offset(-240)); //-365*86400));
        
        mutex_.unlock();
    }
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
    std::cout << "Press Enter to stop...";
    std::cin.get();

    return 0;
}
