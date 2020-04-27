/// server.h
/// Copyright 2020 Cloud-fantasy team
#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include "thread_pool.h"
#include "tcp_socket.h"

namespace simple_http_server
{

/// HTTP server class.
class Server
{
public:
    static const std::string server_name;
    Server(const std::string &ip, uint16_t port, size_t n_threads = 8);

    void start();
    void serve_client(std::unique_ptr<TCPSocket> client_sock);

private:
    std::unique_ptr<TCPSocket> sock;
    thread_pool workers;
};

} // namespace simple_http_server


#endif