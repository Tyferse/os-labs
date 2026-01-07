#include "my_serial.hpp"
#include <iostream>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>


template<class T> std::string to_string(const T& v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: temp_emulator [port]\n";
        return 1;
    }

    cplib::SerialPort port;
    cplib::SerialPort::Parameters params(cplib::SerialPort::BAUDRATE_115200);
    if (port.Open(to_string(argv[1]), params) != cplib::SerialPort::RE_OK) {
        std::cerr << "Failed to open port: " << argv[1] << "\n";
        return 1;
    }

    port.SetTimeout(1.0);
    std::cout << "Emulator started on " << argv[1] << "\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(18.0, 28.0); // Температура от 18 до 28 градусов

    while (true) {
        double temp = dist(gen);
        std::string data = to_string(temp) + "\n";
        
        port << data;
        std::cout << "Sent: " << data;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout.flush();
        port.Flush();
    }

    return 0;
}
