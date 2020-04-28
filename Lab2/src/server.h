/// server.h
/// Copyright 2020 Cloud-fantasy team
#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include <functional>
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
    Server(const std::string &ip, uint16_t port, 
            std::string const &content_base = "", size_t n_threads = 8);

    void start();
    void serve_client(std::unique_ptr<TCPSocket> client_sock);

private:
    std::string &trim_whitespace(std::string &s);
    bool parse_req_line(std::string &line, 
                        std::string &method,
                        std::string &uri,
                        std::string &version);
    std::unique_ptr<Headers> parse_headers(std::string &line);
    /// Parse [uri] and return a file name.
    std::string parse_uri(std::string const &uri);


    /* Error handlers. */
    void page_not_found(std::unique_ptr<TCPSocket> client_sock);
    void method_not_supported(std::unique_ptr<TCPSocket> client_sock, std::string &m);
    void version_not_supported(std::unique_ptr<TCPSocket> client_sock);
    void internal_error(std::unique_ptr<TCPSocket> client_sock, std::string const &msg);

    void handle_get(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock);
    void handle_post(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock);
private:
    friend class thread_pool;

    /// Listen fd.
    std::unique_ptr<TCPSocket> sock;
    /// Pool of workers.
    thread_pool workers;

    using Handlers = std::unordered_map<
        std::string, 
        std::function<void(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket>)> >;

    /// Base dir to serve content.
    std::string content_base;

    /// Handlers.
    Handlers get_handlers;
    Handlers post_handlers;
};

} // namespace simple_http_server


#endif