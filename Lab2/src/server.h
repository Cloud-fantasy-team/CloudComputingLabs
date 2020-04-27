#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include "tcp_socket.h"

namespace simple_http_server
{

class Server
{
public:
    static const std::string server_name;
    Server(const std::string &ip, uint16_t port);

    void start();

private:
    std::unique_ptr<TCPSocket> sock;
};

} // namespace simple_http_server


#endif