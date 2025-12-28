#include "bg_runner.hpp"
#include <iostream>


int main(int argc, char* argv[]) {
    ProcessHandle handle;
    const char* cmd;
    
    if (argc == 0) {
        cmd = "sleep 2";

#ifdef _WIN32
        cmd = "timeout 2";
#endif
    }
    else {
        std::string buf = argv[0];
        for (int i = 1; i < argc; i++) {
            buf += " ";
            buf += argv[i];
        }

        cmd = buf.c_str();
    }

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