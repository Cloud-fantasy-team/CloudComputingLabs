#include <iostream>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "tcp_socket.h"
#include "reporter.h"

namespace simple_http_server
{

TCPSocket::TCPSocket()
{
    socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0)
        report(ERROR) << "failed creating new socket" << std::endl;
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

    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) != 1)
        goto FAIL_CONNECT;

    if (::connect(socket_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
        goto FAIL_CONNECT;

    return true;

  FAIL_CONNECT:
    report(ERROR) << "failed connecting " << ip << ":" << port << std::endl;
    return false;
}

bool TCPSocket::bind(const std::string &ip, uint16_t port)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) != 1)
        goto FAIL_CONNECT;

    if (::bind(socket_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
        goto FAIL_CONNECT;

    return true;
  FAIL_CONNECT:
    report(ERROR) << "failed binding on " << ip << ":" << port << std::endl;
    return false;
}

bool TCPSocket::listen(int backlog)
{
    if (::listen(socket_, backlog) == 0)
        return true;

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