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

static std::string concat(std::vector<std::string> &strs, std::string const &delim)
{
    assert(!strs.empty());
    std::stringstream ss;

    ss << strs[0];
    for (auto iter = strs.begin() + 1; iter != strs.end(); iter++)
    {
        ss << delim << *iter;
    }

    return ss.str();
}

const std::string Server::server_name = "Cloud-fantasy server";

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

        if (kv.size() > 2)
        {
            auto value = std::vector<std::string>(kv.begin() + 1, kv.end());
            kv[1] = concat(value, ":");
        }
        else if (kv.size() < 2)
        {
            report(ERROR) << "invalid header: " << hd << std::endl;
            continue;
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
        page_not_found(std::move(client_sock));
    else
        page_not_found(std::move(client_sock));
}

void Server::handle_get(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock)
{
    
}

void Server::handle_post(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock)
{

}

void Server::page_not_found(std::unique_ptr<TCPSocket> client_sock)
{
    std::unique_ptr<Response> res(new Response());
    res->status_code = 404;
    res->status = "Page Not Found";

    std::stringstream ss;
    ss << "<html><title>404 Not Found</title>";
    ss << "<body bgcolor=\"FFFFFF\">\r\n";
    ss << "<p>Page Not Found</p>";
    ss << "</body></html>";
    std::string body = ss.str();

    res->headers->insert(std::make_pair("Server", server_name));
    res->headers->insert(std::make_pair("Content-type", "text/html"));
    res->headers->insert(std::make_pair("Content-length", std::to_string(body.length())));
    res->body = std::vector<char>(body.begin(), body.end());

    std::string html = res->serialize();
    client_sock->send(html.c_str(), html.length());
    client_sock->close();
}

void Server::internal_error(std::unique_ptr<TCPSocket> client_sock, std::string &msg)
{
    std::unique_ptr<Response> res(new Response());
    res->status_code = 500;
    res->status = "Internal server error";

    res->headers->insert(std::make_pair("Server", server_name));
    res->headers->insert(std::make_pair("Content-type", "text/html"));
    res->headers->insert(std::make_pair("Content-length", std::to_string(msg.length())));
    res->body = std::vector<char>(msg.begin(), msg.end());
}

void Server::version_not_supported(std::unique_ptr<TCPSocket> client_sock)
{
    std::stringstream ss;
    ss << "<html><title>HTTP version not supported</title>";
    ss << "<body>\r\n" << "<p>Unsupported HTTP version</p>";
    ss << "</body></html>";
    std::string body = ss.str();

    internal_error(std::move(client_sock), body);
}

void Server::method_not_supported(std::unique_ptr<TCPSocket> client_sock)
{
    std::stringstream ss;
    ss << "<html><title>method not supported</title>";
    ss << "<body>\r\n" << "<p>Unsupported HTTP method</p>";
    ss << "</body></html>";
    std::string body = ss.str();

    internal_error(std::move(client_sock), body);
}

} // namespace simple_http_serve
