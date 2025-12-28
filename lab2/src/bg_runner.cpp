#include "bg_runner.hpp"
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
    #include <tchar.h>
#else
    #include <sys/wait.h>
    #include <errno.h>
#endif


int run_bg_program(const char* command, ProcessHandle* handle) {
#ifdef _WIN32
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&handle->pi, sizeof(handle->pi));

    if (!CreateProcess(
        NULL, 
        (LPSTR)command, 
        NULL, 
        NULL, 
        FALSE, 
        0, 
        NULL, 
        NULL, 
        &si, 
        &handle->pi)
    ) {
        return -1;
    }
    handle->finished = false;
    return 0;
#else
#endif
}

int wait_for_process(ProcessHandle* handle) {
#ifdef _WIN32
    DWORD result = WaitForSingleObject(handle->pi.hProcess, INFINITE);
    if (result == WAIT_OBJECT_0) {
        GetExitCodeProcess(handle->pi.hProcess, (LPDWORD)&handle->exit_code);
        CloseHandle(handle->pi.hProcess);
        CloseHandle(handle->pi.hThread);
        handle->finished = true;
        return handle->exit_code;
    }
    return -1;
#else
#endif
}
