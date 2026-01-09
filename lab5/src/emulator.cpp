#include "my_serial.hpp"
#include <iostream>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>


// Статистические данные о температуре во Владивостоке по месяцам
// Максимум, Минимум, Среднее, Стандартное отклонение
double history_temper[12][4] = {
    {3.9, -27.6, -12.41, 6.92},
    {8.5, -16.2, -6.01, 4.21},
    {16.9, -6.6, 1.61, 3.18},
    {18.0, -0.9, 6.45, 2.24},
    {24.6, 5.9, 12.36, 2.82},
    {28.0, 10.8, 16.65, 2.7},
    {30.6, 15.3, 20.56, 3.73},
    {29.8, 16.8, 20.85, 2.96},
    {25.4, 8.2, 18.49, 2.69},
    {21.7, 0.6, 11.11, 3.58},
    {13.0, -14.0, -1.51, 5.69},
    {5.4, -22.7, -9.99, 7.43}
};

double gen_temper(std::mt19937& rng) {
    // Получаем текущий месяц
    unsigned month = static_cast<unsigned>(
        std::chrono::year_month_day(
            std::chrono::floor<std::chrono::days>(
                std::chrono::system_clock::now()
            )
        ).month()
    ) - 1;

    std::normal_distribution<> dist(history_temper[month][2], history_temper[month][3]);
    double result = dist(rng);

    result = result < history_temper[month][1] ? history_temper[month][1] : result;
    result = result > history_temper[month][0] ? history_temper[month][0] : result;
    return result;
}

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
    // std::uniform_real_distribution<> dist(18.0, 28.0); // Температура от 18 до 28 градусов

    while (true) {
        double temp = gen_temper(gen); // dist(gen);
        std::string data = to_string(temp) + " " + to_string(temp);
        
        port << data;
        std::cout << "Sent: " << temp << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout.flush();
        port.Flush();
    }

    return 0;
}
