#pragma once

#include "my_serial.hpp"
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <mutex>
#include <thread>
#include <memory>


std::string get_curr_time() {
    auto time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
    std::stringstream strstr;
    strstr << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return strstr.str();
}


class TemperLogger
{
public:
    TemperLogger(const std::string& port_name);
    ~TemperLogger();

    bool start();
    void stop();

private:
    void read_loop();
    void process_temper(double temp);
    void avg_per_hour();
    void avg_per_day();

    cplib::SerialPort port_;
    std::string port_name_;
    std::ofstream full_log_;
    std::ofstream hour_log_;
    std::ofstream day_log_;

    std::vector<double> hour_temps_;
    std::vector<double> day_temps_;

    std::thread read_thread_;
    std::thread hour_thread_;
    std::thread day_thread_;

    bool running_ = false;
    mutable std::mutex mutex_;
};
