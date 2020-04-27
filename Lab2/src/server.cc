#include "server.h"

namespace simple_http_server
{
    
Server::Server(std::string const &ip, uint16_t port)
    : sock(new TCPSocket())
{
    sock->bind(ip, port);
}

void Server::start()
{
    sock->listen(1024);

    for (;;)
    {
        auto sock_client = std::unique_ptr<TCPSocket>(new TCPSocket());
        std::string client_ip;
        uint16_t client_port;
        if (!sock->accept(*sock_client, client_ip, client_port))
        {
            
        }
    }
}

} // namespace simple_http_serve
