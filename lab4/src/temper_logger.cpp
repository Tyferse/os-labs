#include "temper_logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>

#define LOG_DIR std::string("logs/")
#define FULL_LOG std::string("full_temperature.log")
#define HOUR_LOG std::string("hour_temperature.log")
#define DAY_LOG std::string("day_temperature.log")


TemperLogger::TemperLogger(const std::string& port_name)
    : port_name_(port_name) {
    cplib::SerialPort::Parameters params(cplib::SerialPort::BAUDRATE_115200);
    port_.Open(port_name_, params);
    port_.SetTimeout(1.0);

    full_log_.open(LOG_DIR + FULL_LOG, std::ios::app);
    hour_log_.open(LOG_DIR + HOUR_LOG, std::ios::app);
    day_log_.open(LOG_DIR + DAY_LOG, std::ios::app);
}

TemperLogger::~TemperLogger() {
    stop();

    if (full_log_.is_open())
        full_log_.close();
    if (full_log_.is_open())
        hour_log_.close();
    if (full_log_.is_open())
        day_log_.close();
}

bool TemperLogger::start() {
    if (!port_.IsOpen()) {
        std::cerr << "Failed to open port: " << port_name_ << "\n";
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
            double temper;
            if (iss >> temper) {
                process_temper(temper);
            }
        }
    }
}

void TemperLogger::process_temper(double temper) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!full_log_.is_open()) 
        return;

    full_log_ << get_curr_time() << " | " << temper << std::endl;
    full_log_.flush();

    hour_temps_.push_back(temper);
}

void TemperLogger::clear_logs(const std::string& filename, std::time_t threshold_time) {
    std::ifstream in(filename);
    if (!in.is_open()) 
        return;

    std::string temp_filename = filename + ".tmp";
    std::ofstream out(temp_filename);

    std::string line;
    while (std::getline(in, line)) {
        auto timestamp = parse_time(line);
        if (timestamp >= threshold_time) 
            out << line << std::endl;
    }

    in.close();
    out.close();

    // Заменяем оригинальный файл
    std::rename(temp_filename.c_str(), filename.c_str());
}

void TemperLogger::avg_per_hour() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::minutes(60));

        std::lock_guard<std::mutex> lock(mutex_);
        if (hour_temps_.empty()) 
            continue;

        double sum = 0.0;
        for (double t : hour_temps_) 
            sum += t;

        double avg = sum / hour_temps_.size();
        
        if (hour_log_.is_open())
        hour_log_ << get_curr_time() << " | " << avg << std::endl;
        hour_log_.flush();

        hour_temps_.clear();
    }
}

void TemperLogger::avg_per_day() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::hours(24));

        std::lock_guard<std::mutex> lock(mutex_);
        if (day_temps_.empty()) 
            continue;

        double sum = 0.0;
        for (double t : day_temps_) 
            sum += t;

        double avg = sum / day_temps_.size();

        day_log_ << get_curr_time() << " | " << avg << "\n";
        day_log_.flush();

        day_temps_.clear();
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: temper_logger [port]\n";
        return 1;
    }

    TemperLogger logger(argv[1]);
    if (!logger.start()) {
        std::cerr << "Failed to start logger\n";
        return 1;
    }

    std::cout << "Logger started on port: " << argv[1] << "\n";
    std::cout << "Press Enter to stop...\n";
    std::cin.get();

    return 0;
}
