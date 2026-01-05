#pragma once

#include "shared_mem.hpp"
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
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

class CloneLogger {
    public:
        CloneLogger(const std::string& log_file);
        ~CloneLogger();

        void run();

    private:
        void write_log(const std::string& message);
        std::string get_curr_time();
        std::string get_curr_time_ms();
        int get_pid();
        void timerThread();
        void loggingThread();
        void cloningThread();
        void processUserInput();
        void set_counter(int value);
        void spawn_clone(int copy_num);
        bool is_process_running(int pid);
        
        std::string log_file_;
        cplib::SharedMem<CounterData> shared_mem_;
        std::atomic<bool> running_;
        std::thread timer_thread_;
        std::thread logging_thread_;
        std::thread cloning_thread_;
        std::thread input_thread_;
        std::vector<int> spawned_pids_;
        std::atomic<int> current_pid_;
};

#endif