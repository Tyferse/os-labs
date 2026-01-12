#pragma once

#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET closesocket
    #define SEND_FLAGS 0
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    typedef int socket_t;
    #define CLOSE_SOCKET close
    #define SEND_FLAGS MSG_NOSIGNAL
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif


class TCPServer {
    public:
        using RequestHandler = std::function<std::string(const std::string& request)>;
        
        explicit TCPServer(int port, RequestHandler handler);
        ~TCPServer();

        void start();
        void stop();

    private:
        int port_;
        RequestHandler handler_;
        std::atomic<bool> running_{false};
        std::thread server_thread_;
        socket_t listen_socket_ = INVALID_SOCKET;

        void run();
        std::string parse_request_line(const std::string& req);
};
