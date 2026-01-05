#ifndef BG_RUNNER_H
#define BG_RUNNER_H

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/wait.h>
    #include <unistd.h>
    #include <signal.h>
#endif

struct ProcessHandle {
#ifdef _WIN32
    PROCESS_INFORMATION pi;
#else
    pid_t pid;
#endif
    int exit_code;
    bool finished;
};

int run_bg_program(const char* command, ProcessHandle* handle);
int wait_for_process(ProcessHandle* handle);

#endif