/// tcp_socket.h
/// Copyright 2020 Cloud-fantasy team

#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <sys/socket.h>
#include <string>

namespace simple_http_server
{

/// TCP socket class for convenience.
class TCPSocket
{
private:
    /// Underlying socket
    int socket_;

    /// Disallow copy.
    TCPSocket(const TCPSocket &) = delete;
    TCPSocket(const TCPSocket&&) = delete;
    void operator=(const TCPSocket&) = delete;
    void operator=(const TCPSocket&&) = delete;

public:
    TCPSocket();
    ~TCPSocket();

    /// Connection establishment.
    bool connect(const std::string &ip, uint16_t port);
    bool bind(const std::string &ip, uint16_t port);
    bool listen(int backlog = 1024);
    bool accept(TCPSocket &socket, std::string &client_ip, uint16_t &client_port);

    /// [flag] set to true to make socket non-blocking.
    bool set_blocking(bool flag);
    bool shutdown(int how);
    void close();

    /* 
    Send and receive robustly(unbuffered).
    See CSAPP chapter 10.
    */

    /// Send [len] bytes from data robustly.
    int send(const void *data, size_t len);
    /// Send [data] + "\n" robustly.
    int send_line(std::string const &data);
    /// Receive some bytes <= buffer_size.
    int recv(void *buffer, size_t buffer_size);
    /// Receive [n] bytes.
    int recv_bytes(void *buffer, size_t n);
    /// Receive a line with "\n" stripped.
    std::string recv_line();
};

}   // namespace simple_http_server

#endif