#pragma once

#include "shared_mem.hpp"
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <sstream>
#include <fstream>
#include <iostream>

#ifndef CLONGER_H
#define CLONGER_H

#ifdef _WIN32
#   include <windows.h>
#   include <process.h>
#else
#   include <sys/wait.h>
#   include <unistd.h>
#   include <signal.h>
#endif


struct CounterData {
    int counter;
    bool master_exists;
    int master_pid;
    
    CounterData() : counter(0), master_exists(false), master_pid(0) {}
};


std::string get_curr_time() {
    auto time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
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


class CloneLogger {
    public:
        CloneLogger(const std::string& log_file);
        ~CloneLogger();

        void run();

    private:
        void write_log(const std::string& message);
        int get_pid();
        void timerThread();
        void loggingThread();
        void cloningThread();
        void processUserInput();
        void set_counter(int value);
        void spawn_clone(int copy_num);
        bool is_process_running(int pid);
        
        std::string log_file_;
        std::ofstream log_stream_;
        cplib::SharedMem<CounterData> shared_mem_;
        std::atomic<bool> running_;
        std::thread timer_thread_;
        std::thread logging_thread_;
        std::thread cloning_thread_;
        std::thread input_thread_;
        const int current_pid_;
};

#endif