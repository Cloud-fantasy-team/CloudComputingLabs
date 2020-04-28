/// tcp_socket.cc
/// Copyright 2020 Cloud-fantasy team

#include <sstream>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include "tcp_socket.h"
#include "reporter.h"

namespace simple_http_server
{

TCPSocket::TCPSocket()
{
    socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0)
        report(ERROR) << strerror(errno) << std::endl;

    // Prevent "Address in use".
    int optval = 1;
    ::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void*>(&optval), sizeof(optval));
}

TCPSocket::~TCPSocket()
{
    close();
}

bool TCPSocket::connect(const std::string &ip, uint16_t port)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    std::string func;
    if ((inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) != 1) ||
        (::connect(socket_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0))
    {
        report(ERROR) << "failed connecting " << ip << ":" << port << " " << func << std::endl;
        return false;
    }

    report(INFO) << "connecting to " << ip << ":" << port << std::endl;
    return true;
}

bool TCPSocket::bind(const std::string &ip, uint16_t port)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if ((inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) != 1) ||
        (::bind(socket_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0))
    {
        report(ERROR) << "fail binding to " << ip << ":" << port << std::endl;
        return false;
    }

    report(INFO) << "bound to " << ip << ":" << port << std::endl;
    return true;
}

bool TCPSocket::listen(int backlog)
{
    if (::listen(socket_, backlog) == 0)
    {
        report(INFO) << "listening" << std::endl;
        return true;
    }

    report(ERROR) << "failed listening on socket " << socket_ << std::endl;
    return false;
}

bool TCPSocket::accept(TCPSocket &sock, std::string &ip, uint16_t &port)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int sock_client = ::accept(socket_, reinterpret_cast<struct sockaddr*>(&client_addr), &len);
    if (sock_client < 0)
    {
        report(ERROR) << "fail accepting connection" << std::endl;
        return false;
    }

    char tmp[INET_ADDRSTRLEN];
    const char *client_ip = inet_ntop(AF_INET, &client_addr.sin_addr, tmp, INET_ADDRSTRLEN);
    if (!client_ip)
    {
        report(ERROR) << "fail converting network address" << std::endl;
        return false;
    }

    ip.assign(client_ip);
    port = ntohs(client_addr.sin_port);
    sock.socket_ = sock_client;

    report(INFO) << "accepting " << ip << ":" << port << std::endl;
    return true;
}

bool TCPSocket::set_blocking(bool flag)
{
    int opts;
    if ((opts = fcntl(socket_, F_GETFL)) < 0)
    {
        report(ERROR) << "fail getting socket status" << std::endl;
        return false;
    }

    if (flag)
        opts |= O_NONBLOCK;
    else
        opts &= ~O_NONBLOCK;

    if (fcntl(socket_, F_SETFL, opts) < 0)
    {
        report(ERROR) << "fail setting socket status" << std::endl;
        return false;
    }

    return true;
    
}

/// Send [len] bytes from data robustly.
int TCPSocket::send(const void *data, size_t len)
{
    const char *data_buf = reinterpret_cast<const char*>(data);
    std::size_t data_len = len;

    while (data_len > 0)
    {
        int n = ::send(socket_, data_buf, data_len, 0);
        if (n <= 0)
        {
            if (errno == EINTR)
                n = 0;
            else
                return -1;
        }

        data_len -= n;
        data_buf += n;
    }

    return (len - data_len);
}

int TCPSocket::send_line(std::string const &data)
{
    return send((data + "\n").c_str(), data.length() + 1);
}

int TCPSocket::recv(void *buffer, size_t buffer_size)
{
    char *data_buf = reinterpret_cast<char*>(buffer);
    int n = 0;

    // Make sure we'll always recv'ed something from socket_.
    while (n == 0)
    {
        n += ::recv(socket_, data_buf, buffer_size, 0);
        if (n < 0)
        {
            if (errno == EINTR)
                n = 0;
            else
                return -1;
        }
        else if (n == 0)
            return 0;
    }

    return n;
}

int TCPSocket::recv_bytes(void *buffer, size_t n)
{
    size_t nleft = n;
    char *data_buf = reinterpret_cast<char*>(buffer);

    while (nleft > 0)
    {
        ssize_t n = recv(data_buf, nleft);
        if (n < 0)
            return -1;
        else if (n == 0)
            // EOF.
            break;
        nleft -= n;
        data_buf += n;
    }

    return (n - nleft);
}

std::string TCPSocket::recv_line()
{
    std::stringstream ss;
    char buffer[2];

    for (;;)
    {
        int n = recv(buffer, 1);
        if (n == 1)
        {
            if (*buffer == '\n')
                break;

            ss << *buffer;
        }
        else if (n == 0)
            break;
        else
            // Error reading a line.
            return "";
    }
    return ss.str();
}

bool TCPSocket::shutdown(int how)
{
    return (::shutdown(socket_, how) == 0);
}

void TCPSocket::close()
{
    if (socket_ >= 0)
    {
        ::close(socket_);
        socket_ = -1;
    }
}

} // namespace simple_http_server