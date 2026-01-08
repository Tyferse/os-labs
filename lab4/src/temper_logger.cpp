#include "temper_logger.hpp"
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

#define LOG_DIR std::string("logs/")
#define FULL_LOG std::string("full_temperature.log")
#define HOUR_LOG std::string("hour_temperature.log")
#define DAY_LOG std::string("day_temperature.log")

enum LOG_IDS {FULL_F, HOUR_F, DAY_F};
std::string LOG_FILES[3] = {LOG_DIR + FULL_LOG, LOG_DIR + HOUR_LOG, LOG_DIR + DAY_LOG};


TemperLogger::TemperLogger(const std::string& port_name)
    : port_name_(port_name) {
    cplib::SerialPort::Parameters params(cplib::SerialPort::BAUDRATE_115200);
    port_.Open(port_name_, params);
    port_.SetTimeout(1.0);

    if (!directory_exists(LOG_DIR))
        create_single_directory(LOG_DIR);
    
    full_log_.open(LOG_FILES[FULL_F], std::ios::app);
    hour_log_.open(LOG_FILES[HOUR_F], std::ios::app);
    day_log_.open(LOG_FILES[DAY_F], std::ios::app);
}

TemperLogger::~TemperLogger() {
    stop();

    if (full_log_.is_open())
        full_log_.close();
    if (hour_log_.is_open())
        hour_log_.close();
    if (day_log_.is_open())
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

void TemperLogger::clear_logs(unsigned int file_, std::time_t threshold_time) {
    std::ifstream in(LOG_FILES[file_]);
    if (!in.is_open()) 
        return;

    std::string temp_filename = LOG_FILES[file_] + ".tmp";
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
#ifdef _WIN32
    if (!MoveFileExA(temp_filename.c_str(), LOG_FILES[file_].c_str(), MOVEFILE_REPLACE_EXISTING)) {
        DWORD err = GetLastError();
        // Можно бросить исключение или обработать ошибку
        std::cerr << "MoveFileEx failed with error: " << err << std::endl;
    }
#else
    if (std::rename(temp_filename.c_str(), LOG_FILES[file_].c_str()) != 0) {
        perror("rename");
    }
#endif 
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

    // Если массив пуст, считаем, что прошёл час, или это начало работы программы
    if (hour_temps_.empty()) {
        if (full_log_.is_open())
            full_log_.close();
        
        clear_logs(FULL_F, subtract_seconds(std::time(nullptr), 60));
        full_log_.open(LOG_FILES[FULL_F], std::ios::app);
    }

    full_log_ << get_curr_time() << " | " << temper << std::endl;
    full_log_.flush();

    hour_temps_.push_back(temper);
    day_temps_.push_back(temper);
}

void TemperLogger::avg_per_hour() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::minutes(1));

        mutex_.lock();
        if (hour_temps_.empty() || !hour_log_.is_open()) 
            continue;

        double sum = 0.0;
        for (double t : hour_temps_) 
            sum += t;
        
        hour_log_ << get_curr_time() << " | " << sum / hour_temps_.size() << std::endl;
        hour_log_.flush();

        hour_temps_.clear();
        mutex_.unlock();
    }
}

void TemperLogger::avg_per_day() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::minutes(3)); //hours(24));

        mutex_.lock();
        if (day_temps_.empty() || !day_log_.is_open()) 
            continue;

        // Очистка почасовых и посуточных логов раз в сутки
        if (hour_log_.is_open())
            hour_log_.close();
        
        clear_logs(HOUR_F, subtract_seconds(std::time(nullptr), 3*60));
        hour_log_.open(LOG_FILES[HOUR_F], std::ios::app);

        if (day_log_.is_open())
            day_log_.close();
        
        clear_logs(DAY_F, subtract_seconds(std::time(nullptr), 6*60));
        day_log_.open(LOG_FILES[DAY_F], std::ios::app);
        

        double sum = 0.0;
        for (double t : day_temps_) 
            sum += t;

        day_log_ << get_curr_time() << " | " << sum / day_temps_.size() << "\n";
        day_log_.flush();

        day_log_.clear();
        mutex_.unlock();
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
