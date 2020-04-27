#include <cstdlib>
#include "server.h"
#include "reporter.h"

namespace simple_http_server
{
    
Server::Server(std::string const &ip, uint16_t port, size_t n_threads)
    : sock(new TCPSocket()), workers(*this, n_threads)
{
    sock->bind(ip, port);
}

void Server::start()
{
    sock->listen(1024);

    for (;;)
    {
        std::unique_ptr<TCPSocket> sock_client(new TCPSocket());
        std::string client_ip;
        uint16_t client_port;

        // Abort on failure accepting.
        if (!sock->accept(*sock_client, client_ip, client_port))
            abort();

        // Add it to task pool.
        workers.add_client(std::move(sock_client));
    }
}

void Server::serve_client(std::unique_ptr<TCPSocket> client_sock)
{
    std::cout << "serve_client" << std::endl;
}

} // namespace simple_http_serve
