#pragma once

#include "my_serial.hpp"
#include "db.hpp"
#include "server.hpp"
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <mutex>
#include <thread>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif


bool directory_exists(const std::string& path) {
#ifdef _WIN32
    return _access(path.c_str(), 0) == 0;
#else
    return access(path.c_str(), F_OK) == 0;
#endif
}

bool create_single_directory(const std::string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0777) == 0; 
#endif
}

std::string get_curr_time() {
    auto time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
    std::stringstream strstr;
    strstr << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return strstr.str();
}

std::string get_time_offset(long offset_sec) {
    auto tp = std::chrono::system_clock::now();
    tp += std::chrono::seconds(offset_sec);
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    
    std::stringstream strstr;
    strstr << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return strstr.str();
}

std::string url_decode(const std::string& input) {
    std::string output;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '%' && i + 2 < input.length()) {
            char hex_str[3] = {input[i+1], input[i+2], '\0'};
            char decoded_char = static_cast<char>(std::stoi(hex_str, nullptr, 16));
            output += decoded_char;
            i += 2;
        } else if (input[i] == '+') {
            output += ' ';
        } else {
            output += input[i];
        }
    }
    return output;
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
    std::string handle_tcp_request(const std::string& request);

    cplib::SerialPort port_;
    std::string port_name_;
    
    std::unique_ptr<DBManager> dbm_;
    std::unique_ptr<TCPServer> tcp_server_;

    std::thread read_thread_;
    std::thread hour_thread_;
    std::thread day_thread_;

    bool running_ = false;
    mutable std::mutex mutex_;
};
