#include "bg_runner.hpp"
#include <iostream>


int main() {
    ProcessHandle handle;
    const char* cmd = "sleep 2";

#ifdef _WIN32
    cmd = "timeout 2";
#endif

    std::cout << "Start programm: " << cmd << std::endl;

    if (run_bg_program(cmd, &handle) != 0) {
        std::cerr << "Failed to run process..." << std::endl;
        return 1;
    }

    std::cout << "Waiting for finishing process..." << std::endl;

    int exit_code = wait_for_process(&handle);
    std::cout << "Process has finished with exit code: " << exit_code << std::endl;

    return 0;
}