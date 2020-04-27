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

    /// Avoid copy and assignment.
    TCPSocket(const TCPSocket &);
    void operator=(const TCPSocket&);

public:
    TCPSocket();
    ~TCPSocket();

    /// Connection establishment.
    bool connect(const std::string &ip, uint16_t port);
    bool bind(const std::string &ip, uint16_t port);
    bool listen(int backlog);
    bool accept(TCPSocket &socket, std::string &client_ip, uint16_t &client_port);

    /// [flag] set to true to make socket non-blocking.
    bool set_blocking(bool flag);
    bool shutdown(int how);
    void close();

    /// Send and receive.
    int send(void *data, size_t len);
    int send(std::string &data);
    int receive(void *buffer, int buffer_size);
};

}   // namespace simple_http_server

#endif