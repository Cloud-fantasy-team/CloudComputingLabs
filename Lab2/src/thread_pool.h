#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "tcp_socket.h"

namespace simple_http_server 
{

class Server;

/// A very simple thread pool implementation.
class thread_pool
{
public:
    thread_pool(Server &server, size_t size);
    ~thread_pool();

    void add_client(std::unique_ptr<TCPSocket> sock);

private:
    /// HTTP server.
    Server &server;

    /// Workers themselves.
    std::vector<std::thread> workers;

    /// TCP sockets to serve.
    std::queue<std::unique_ptr<TCPSocket>> client_socks;

    /// Synchronization primitives.
    bool done;
    std::mutex m;
    std::condition_variable condition;
};

}   // namespace simple_http_server
#endif