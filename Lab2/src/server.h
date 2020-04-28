/// server.h
/// Copyright 2020 Cloud-fantasy team
#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include "thread_pool.h"
#include "tcp_socket.h"
#include "message.h"

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
    std::string &trim_whitespace(std::string &s);
    bool parse_req_line(std::string &line, 
                        std::string &method,
                        std::string &uri,
                        std::string &version);
    std::unique_ptr<Headers> parse_headers(std::string &line);

    void method_not_supported(std::unique_ptr<TCPSocket> client_sock);
    void version_not_supported(std::unique_ptr<TCPSocket> client_sock);
    void internal_error(std::unique_ptr<TCPSocket> client_sock);

    void handle_get(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock);
    void handle_post(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock);
private:
    friend class thread_pool;

    std::unique_ptr<TCPSocket> sock;
    thread_pool workers;
};

} // namespace simple_http_server


#endif