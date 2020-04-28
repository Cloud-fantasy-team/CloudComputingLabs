/// server.h
/// Copyright 2020 Cloud-fantasy team

#include <sstream>
#include <cstdlib>
#include "server.h"
#include "reporter.h"

namespace simple_http_server
{

/// Simple string splitter.
static std::vector<std::string> split(std::string const &str, std::string const &delim)
{
    std::vector<std::string> tokens;
    std::string str_copy = str;
    std::string word;
    std::size_t pos;

    while ((pos = str_copy.find(delim)) != std::string::npos)
    {
        word = str_copy.substr(0, pos);
        str_copy.erase(0, pos + delim.length());
        tokens.emplace_back(word);
    }

    if (str_copy.length())
        tokens.emplace_back(str_copy);

    return tokens;
}

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

std::string &Server::trim_whitespace(std::string &s)
{
    if (s.empty())  return s;
    while (s[0] == ' ' || s[0] == '\t' || s[0] == '\r' || s[0] == '\n')
        s = s.substr(1, s.length());

    if (s.empty())  return s;
    char ch = s.back();
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
    {
        s = s.substr(0, s.length() - 1);
        ch = s.back();
    }

    return s;
}

bool Server::parse_req_line(std::string &line, 
                            std::string &method,
                            std::string &uri,
                            std::string &version)
{
    trim_whitespace(line);
    std::vector<std::string> v = split(line, " ");
    if (v.size() != 3)
    {
        report(ERROR) << "invalid http request: " << line << std::endl;
        return false;
    }

    method = v[0];
    uri = v[1];
    version = v[2];
    return true;
}

std::unique_ptr<Headers> Server::parse_headers(std::string &str)
{
    auto hdr_obj = std::unique_ptr<Headers>(new Headers());
    auto hdrs = split(str, "\n");

    for (auto &hd : hdrs)
    {
        trim_whitespace(hd);
        auto kv = split(hd, ":");

        if (kv.size() != 2)
        {
            report(ERROR) << "invalid header" << std::endl;
            abort();
        }

        trim_whitespace(kv[0]);
        trim_whitespace(kv[1]);
        (*hdr_obj)[kv[0]] = kv[1];
    }
    return hdr_obj;
}

void Server::serve_client(std::unique_ptr<TCPSocket> client_sock)
{
    std::unique_ptr<Request> req(new Request());
    std::string req_line = client_sock->recv_line();

    /* request line. */
    if (!parse_req_line(req_line, req->method, req->resource, req->version))
    {
        client_sock->close();
        return;
    }

    if (req->version != "HTTP/1.1")
    {
        version_not_supported(std::move(client_sock));
        return;
    }

    /* headers. */
    std::stringstream header_stream;
    std::string line = client_sock->recv_line();
    while (line != "\r\n")
    {
        header_stream << line;
        line = client_sock->recv_line();
    }

    std::string header_str = header_stream.str();
    std::unique_ptr<Headers> headers = parse_headers(header_str);
    if (!headers)
    {
        internal_error(std::move(client_sock));
        return;
    }
    req->headers.reset(headers.release());

    /* dispatch. */
    if (req->method == "GET")
        handle_get(std::move(req), std::move(client_sock));
    else if (req->method == "POST")
        handle_post(std::move(req), std::move(client_sock));
    else
        method_not_supported(std::move(client_sock));
}

} // namespace simple_http_serve
