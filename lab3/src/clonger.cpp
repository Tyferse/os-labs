#include "clonger.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#define SHARED_MEM_NAME "counter_mem"
#define LOG_FILE "log.log"

#ifdef _WIN32
#   include <windows.h>
#   include <io.h>
#else
#   include <unistd.h>
#endif


std::string get_curr_time() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream strstr;
    strstr << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return strstr.str();
}

std::string get_curr_time_ms() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream strstr;
    strstr << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    strstr << "." << std::setfill('0') << std::setw(3) << ms.count();
    return strstr.str();
}


CloneLogger::CloneLogger(const std::string& log_file)
    : log_file_(log_file), log_stream_(log_file, std::ios::app), 
      shared_mem_(SHARED_MEM_NAME), current_pid_(get_pid()), running_(true)
{
    if (!shared_mem_.IsValid()) {
        std::cout << "Failed to create shared memory block!" << std::endl;
        exit(1);
    }
    else {
        shared_mem_.Data()->master_exists = false;
        shared_mem_.Data()->master_pid = 0;
        shared_mem_.Data()->counter = 0;
    }

    // current_pid_ = get_pid();
    std::cout << current_pid_ << std::endl;

    write_log("Process started with PID: " + std::to_string(current_pid_) 
              + " at " + get_curr_time());
}

CloneLogger::~CloneLogger() {
    running_ = false;
    
    if (timer_thread_.joinable()) 
        timer_thread_.join();
    if (logging_thread_.joinable()) 
        logging_thread_.join();
    if (cloning_thread_.joinable()) 
        cloning_thread_.join();
    if (input_thread_.joinable()) 
        input_thread_.join();
    
    write_log("Process with PID: " + std::to_string(current_pid_) 
              + " finished at " + get_curr_time());

    if (log_stream_.is_open()) {
        log_stream_.close();
    }
}

void CloneLogger::run() {
    timer_thread_ = std::thread(&CloneLogger::timerThread, this);
    logging_thread_ = std::thread(&CloneLogger::loggingThread, this);
    cloning_thread_ = std::thread(&CloneLogger::cloningThread, this);
    input_thread_ = std::thread(&CloneLogger::processUserInput, this);
    
    timer_thread_.join();
    logging_thread_.join();
    cloning_thread_.join();
    input_thread_.join();
}

void CloneLogger::write_log(const std::string& message) {
    // std::ofstream log(log_file_, std::ios::app);
    if (log_stream_.is_open()) {
        log_stream_ << get_curr_time_ms() << " | PID: " << current_pid_ 
            << " | " << message << std::endl;
        // log.close();
    }
}

int CloneLogger::get_pid() {
#ifdef _WIN32
    return static_cast<int>(GetCurrentProcessId());
#else
    return static_cast<int>(getpid());
#endif
}

void CloneLogger::timerThread() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        shared_mem_.Lock();
        CounterData* data = shared_mem_.Data();
        if (data) {
            // std::cout << data->master_exists << " timer " << data->master_pid << std::endl;
            data->counter++;
        }
        shared_mem_.Unlock();
    }
}

void CloneLogger::loggingThread() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        shared_mem_.Lock();
        CounterData* data = shared_mem_.Data();
        if (data)
            // std::cout << data->master_exists << " log " << data->master_pid << std::endl;
            if (data->master_exists && data->master_pid == current_pid_)
                write_log("Counter: " + std::to_string(data->counter));
        
        shared_mem_.Unlock();
    }
}

void CloneLogger::cloningThread() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        shared_mem_.Lock();
        CounterData* data = shared_mem_.Data();
        if (data) {
            // Проверяем, есть ли уже запущенные копии
            bool spawn_allowed = false;
            
            // Проверяем, является ли этот процесс основным
            // std::cout << data->master_exists << " clone " << data->master_pid << std::endl;
            if (!data->master_exists) {
                data->master_exists = true;
                data->master_pid = current_pid_;
                spawn_allowed = true;
                std::cout << data->master_exists << " clone " << data->master_pid << std::endl;
            } else if (data->master_pid == current_pid_) {
                spawn_allowed = true;
            }
            
            if (spawn_allowed) {
                spawn_clone(1);
                spawn_clone(2);
            } else {
                write_log("Another master process is running, skipping spawn");
            }
        }
        shared_mem_.Unlock();
    }
}

void CloneLogger::spawn_clone(int clone_num) {
    std::string clone_arg = "--clone " + std::to_string(clone_num);
    
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::string cmd = "build\\clonger.exe " + clone_arg;
    char* cmd_char = const_cast<char*>(cmd.c_str());
    
    if (CreateProcess(NULL, cmd_char, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        // Дочерний процесс
        // char* args[] = {"./build/clonger", const_cast<char*>(clone_arg.c_str())};
        // execv("./build/clonger", args);
        // execl("./build/clonger", "./build/clonger", clone_arg.c_str(), (char*)NULL);
        execlp("./build/clonger", "./build/cloneger", "--clone", std::to_string(clone_num).c_str(), (char*)NULL);
        _exit(127);
    }
#endif
}

void CloneLogger::processUserInput() {
    std::string input;
    while (running_) {
        std::getline(std::cin, input);
        
        if (input.substr(0, 3) == "set") {
            try {
                size_t pos = input.find(' ');
                if (pos != std::string::npos) {
                    std::string str_value = input.substr(pos + 1);
                    int value = std::stoi(str_value);
                    set_counter(value);
                }
            } 
            catch (...) {
                std::cout << "Invalid counter value!" << std::endl;
            }
        } 
        else if (input == "quit") {
            running_ = false;
            break;

        }
    }
}

void CloneLogger::set_counter(int value) {
    shared_mem_.Lock();
    CounterData* data = shared_mem_.Data();
    if (data)
        data->counter = value;
    
    shared_mem_.Unlock();
}

void run_clone(int clone_num) {
    cplib::SharedMem<CounterData> shared_mem(SHARED_MEM_NAME);
    if (!shared_mem.IsValid()) {
        std::cerr << "Failed to access shared memory in copy!" << std::endl;
        return;
    }
    
    int pid = 0;
#ifdef _WIN32
    pid = static_cast<int>(GetCurrentProcessId());
#else
    pid = static_cast<int>(getpid());
#endif

    std::ofstream log(LOG_FILE, std::ios::app);
    if (log.is_open()) {
        log << get_curr_time_ms() << " | PID: " << pid 
            << " | Clone " << clone_num << " started" << std::endl;
        log.close();
    }

    shared_mem.Lock();
    CounterData* data = shared_mem.Data();
    if (data) {
        if (clone_num == 1) {
            // Copy 1: увеличивает счетчик на 10
            data->counter += 10;
        } else if (clone_num == 2) {
            // Copy 2: увеличивает в 2 раза, через 2 секунды уменьшает в 2 раза
            int old_val = data->counter;
            data->counter *= 2;
        }
    }
    shared_mem.Unlock();

    if (clone_num == 2) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        shared_mem.Lock();
        CounterData* data = shared_mem.Data();
        if (data) {
            data->counter /= 2;
        }
        shared_mem.Unlock();
    }

    log.open(LOG_FILE, std::ios::app);
    if (log.is_open()) {
        log << get_curr_time_ms() << " | PID: " << pid 
            << " | Clone " << clone_num << " finished" << std::endl;
        log.close();
    }
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cout << argv[1] << std::endl;
        std::string arg = argv[1];
        if (arg.substr(0, 7) == "--clone") {
            std::cout << argv[1] << std::endl;
            int clone_num = std::stoi(argv[2]);
            run_clone(clone_num);
            return 0;
        }
    }
    
    // Основной процесс
    CloneLogger app(LOG_FILE);
    app.run();
    return 0;
}
