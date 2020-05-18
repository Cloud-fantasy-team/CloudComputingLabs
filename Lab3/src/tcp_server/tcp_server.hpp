#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <list>
#include "tcp_socket.hpp"
#include "tcp_client.hpp"
#include "reactor.hpp"

namespace tcp_server
{

class tcp_server
{
public:
    tcp_server();
    /// Rule of 5.
    ~tcp_server();
    tcp_server(const tcp_server&) = delete;
    tcp_server &operator=(const tcp_server&) = delete;
    tcp_server(tcp_server&&);
    tcp_server &operator=(tcp_server&&);

public:

private:
    reactor &reactor;

    /// Underlying listening socket.
    tcp_socket socket_;

    /// A list of clients.
    std::list<std::unique_ptr<tcp_client>> clients_;
};

} // namespace tcp_server


#endif