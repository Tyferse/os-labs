#include "temper_logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>

#define LOG_DIR std::string("logs/")


TemperLogger::TemperLogger(const std::string& port_name)
    : port_name_(port_name)
{
    cplib::SerialPort::Parameters params(cplib::SerialPort::BAUDRATE_115200);
    port_.Open(port_name_, params);
    port_.SetTimeout(1.0);

    full_log_.open(LOG_DIR + "full_temperature.log", std::ios::app);
    hour_log_.open(LOG_DIR + "hour_temperature.log", std::ios::app);
    day_log_.open(LOG_DIR + "day_temperature.log", std::ios::app);
}

TemperLogger::~TemperLogger()
{
    stop();

    if (full_log_.is_open())
        full_log_.close();
    if (full_log_.is_open())
        hour_log_.close();
    if (full_log_.is_open())
        day_log_.close();
}

bool TemperLogger::start()
{
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

void TemperLogger::stop()
{
    running_ = false;
    if (read_thread_.joinable()) 
        read_thread_.join();
    if (hour_thread_.joinable()) 
        hour_thread_.join();
    if (day_thread_.joinable()) 
        day_thread_.join();
}

void TemperLogger::read_loop()
{
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

void TemperLogger::process_temper(double temper)
{
    std::lock_guard<std::mutex> lock(mutex_);

    full_log_ << get_curr_time() << " | " << temper << std::endl;
    full_log_.flush();

    hour_temps_.push_back(temper);
}

void TemperLogger::avg_per_hour()
{
    while (running_) {
        std::this_thread::sleep_for(std::chrono::minutes(60));

        std::lock_guard<std::mutex> lock(mutex_);
        if (hour_temps_.empty()) 
            continue;

        double sum = 0.0;
        for (double t : hour_temps_) 
            sum += t;

        double avg = sum / hour_temps_.size();

        hour_log_ << get_curr_time() << " | " << avg << std::endl;
        hour_log_.flush();

        hour_temps_.clear();
    }
}

void TemperLogger::avg_per_day()
{
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


int main(int argc, char* argv[])
{
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
