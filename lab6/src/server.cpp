#include "server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <thread>
#include <algorithm>

#ifdef _WIN32
    static bool winsock_initialized = false;
#endif


TCPServer::TCPServer(int port, RequestHandler handler)
    : port_(port), handler_(std::move(handler)) {
#ifdef _WIN32
    if (!winsock_initialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
        winsock_initialized = true;
    }
#endif
}

TCPServer::~TCPServer() {
    stop();
#ifdef _WIN32
    if (winsock_initialized) {
        WSACleanup();
        winsock_initialized = false;
    }
#endif
}

void TCPServer::start() {
    if (running_) return;
    running_ = true;
    server_thread_ = std::thread(&TCPServer::run, this);
}

void TCPServer::stop() {
    if (!running_) return;
    running_ = false;

    if (listen_socket_ != INVALID_SOCKET) {
        CLOSE_SOCKET(listen_socket_);
        listen_socket_ = INVALID_SOCKET;
    }

    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}

void TCPServer::run() {
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket_ == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_);

    if (bind(listen_socket_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        CLOSE_SOCKET(listen_socket_);
        return;
    }

    if (listen(listen_socket_, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        CLOSE_SOCKET(listen_socket_);
        return;
    }

    std::cout << "TCP server listening on port " << port_ << std::endl;

    while (running_) {
        socket_t client_socket = accept(listen_socket_, (struct sockaddr*)&cli_addr, &clilen);
        if (client_socket == INVALID_SOCKET) {
            if (running_)
                std::cerr << "Accept failed" << std::endl;
            
            continue;
        }

        std::thread([this, client_socket]() {
            char buffer[4096];
            int n = recv(client_socket, buffer, sizeof(buffer)-1, 0);
            if (n > 0) {
                buffer[n] = '\0';
                std::string request(buffer);

                std::string response = handler_(request);
                std::string full_response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                            std::to_string(response.length()) + "\r\n\r\n" + response;

                send(client_socket, full_response.c_str(), full_response.length(), SEND_FLAGS);
            }
            CLOSE_SOCKET(client_socket);
        }).detach();
    }

    CLOSE_SOCKET(listen_socket_);
    listen_socket_ = INVALID_SOCKET;
}

std::string TCPServer::parse_request_line(const std::string& req) {
    size_t pos = req.find("\r\n");
    if (pos != std::string::npos) {
        std::string first_line = req.substr(0, pos);
        size_t start = first_line.find(' ');
        size_t end = first_line.find(' ', start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            return first_line.substr(start + 1, end - start - 1);
        }
    }
    return "";
}
