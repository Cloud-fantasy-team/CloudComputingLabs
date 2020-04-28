/// server.h
/// Copyright 2020 Cloud-fantasy team

#include <sstream>
#include <cstdlib>
#include <unistd.h>
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

static std::string &trim_trailing_slash(std::string &s)
{
    if (s.empty())  return s;
    while (s.back() == '/')
        s = s.substr(0, s.length() - 1);
    return s;
}

const std::string Server::server_name = "Cloud-fantasy server";

Server::Server(std::string const &ip, uint16_t port, 
                std::string const &content_base, size_t n_threads)
    : sock(new TCPSocket()), workers(*this, n_threads)
{
    if (content_base == "")
    {
        char cwd_buf[1024];
        this->content_base = std::string(getcwd(cwd_buf, sizeof(cwd_buf)));
    }
    else
    {
        // Check for existence.
        {
            std::ifstream test_existence(content_base);
            if (!test_existence)
            {
                report(ERROR) << "Non-existed content base dir: " << content_base << std::endl;
                abort();
            }
        }

        this->content_base = content_base;
        trim_trailing_slash(this->content_base);
    }
    
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

/// A complete ugly hack since I'm running out of time.
std::string Server::parse_uri(std::string const &uri)
{
    std::string f = content_base;
    std::string normalized = uri;

    if (normalized.length() > 2 && normalized[0] == '.' && normalized[1] == '/')
        normalized = normalized.substr(1);

    if (normalized.length() && normalized[0] != '/')
        return "";

    f += normalized;
    if (f.back() == '/')
        f += "index.html";

    return f;
}

std::unordered_map<std::string, std::string> 
Server::parse_name_id(std::string const&data)
{
    std::unordered_map<std::string, std::string> map;
    auto pairs = split(data, "&");
    
    for (auto &pair : pairs)
    {
        auto kv = split(pair, "=");
        if (kv.size() != 2)
        {
            report(ERROR) << "Unknown data " << pair << std::endl;
            continue;
        }

        trim_whitespace(kv[0]);
        trim_whitespace(kv[1]);
        if (map.count(kv[0]))
            report(ERROR) << "duplicate data " << pair << " and " << kv[0] << map[kv[0]] << std::endl;
        map[kv[0]] = kv[1];
    }
    return map;
}

/// Complete hack.
void Server::recv_body(Request *req, TCPSocket *client_sock)
{
    static std::vector<std::string> content_lens = {"content-length", "Content-Length", "Content-length"};

    for (auto &CL : content_lens)
        if (req->headers->find(CL) != req->headers->end())
        {
            long long content_len = std::stoll(req->headers->find(CL)->second);
            if (content_len > 0)
            {
                char *buff = new char[content_len];
                client_sock->recv_bytes(buff, content_len);
                req->body = std::vector<char>(buff, buff + content_len);
                delete[] buff;
            }
            return;
        }
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
        internal_error(std::move(client_sock), "internal error");
        return;
    }
    req->headers.reset(headers.release());

    /* body. */
    recv_body(req.get(), client_sock.get());

    /* dispatch. */
    if (req->method == "GET")
        handle_get(std::move(req), std::move(client_sock));
    else if (req->method == "POST")
        handle_post(std::move(req), std::move(client_sock));
    else
        method_not_supported(std::move(client_sock), req->method);
}

void Server::handle_get(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock)
{
    std::unique_ptr<Response> res(new Response());
    std::string filename = parse_uri(req->resource);

    std::ifstream file(filename);
    if (!file)
    {
        page_not_found(std::move(client_sock), req->resource);
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    res->status_code = 200;
    res->status = "OK";
    res->body = std::vector<char>(content.begin(), content.end());
    res->headers->insert(std::make_pair("Server", server_name));
    res->headers->insert(std::make_pair("Content-type", "text/html"));
    res->headers->insert(std::make_pair("Content-length", std::to_string(content.length())));

    std::string html = res->serialize();
    client_sock->send(html.c_str(), html.length());
}

/// I know this is ugly. But I'm running out of time.
void Server::handle_post(std::unique_ptr<Request> req, std::unique_ptr<TCPSocket> client_sock)
{
    std::unique_ptr<Response> res(new Response());
    std::string uri = req->resource;
    auto data = parse_name_id(std::string(req->body.begin(), req->body.end()));

    if (uri != "/Post_show" || !data.count("Name") || !data.count("ID"))
    {
        page_not_found(std::move(client_sock), uri);
        return;
    }

    res->status_code = 200;
    res->status = "OK";

    std::stringstream ss;
    ss << "<html><title>POST method</title><body bgcolor=ffffff>\r\n";
    ss << "Your Name:   " << data["Name"] << "\r\n";
    ss << "ID:  " << data["ID"] << "\r\n";
    ss << "<hr><em>Http Web server</em>\r\n";
    ss << "</body></html>\r\n";
    std::string content = ss.str();

    res->headers->insert(std::make_pair("Server", server_name));
    res->headers->insert(std::make_pair("Content-type", "text/html"));
    res->headers->insert(std::make_pair("Content-length", std::to_string(content.length())));
    res->body = std::vector<char>(content.begin(), content.end());

    std::string html = res->serialize();
    client_sock->send(html.c_str(), html.length());
    client_sock->close();
}

void Server::page_not_found(std::unique_ptr<TCPSocket> client_sock, std::string const &f)
{
    std::unique_ptr<Response> res(new Response());
    res->status_code = 404;
    res->status = "Page Not Found";

    std::stringstream ss;
    ss << "<html><title>404 Not Found</title><body bgcolor=\"FFFFFF\">\r\n";
    ss << " Not Found\r\n";
    ss << "<p>Couldn't find this file: " << f << "\r\n";
    ss << "<hr><em>HTTP Web server</em>\r\n";
    ss << "</body></html>\r\n";
    std::string body = ss.str();

    res->headers->insert(std::make_pair("Server", server_name));
    res->headers->insert(std::make_pair("Content-type", "text/html"));
    res->headers->insert(std::make_pair("Content-length", std::to_string(body.length())));
    res->body = std::vector<char>(body.begin(), body.end());

    std::string html = res->serialize();
    client_sock->send(html.c_str(), html.length());
    client_sock->close();
}

void Server::internal_error(std::unique_ptr<TCPSocket> client_sock, std::string const &msg)
{
    std::unique_ptr<Response> res(new Response());
    res->status_code = 501;
    res->status = "Not Implemented";

    res->headers->insert(std::make_pair("Server", server_name));
    res->headers->insert(std::make_pair("Content-type", "text/html"));
    res->headers->insert(std::make_pair("Content-length", std::to_string(msg.length())));
    res->body = std::vector<char>(msg.begin(), msg.end());

    std::string html = res->serialize();
    client_sock->send(html.c_str(), html.length());
    client_sock->close();
}

void Server::version_not_supported(std::unique_ptr<TCPSocket> client_sock)
{
    std::stringstream ss;
    ss << "<html><title>HTTP version not supported</title>";
    ss << "<body>\r\n" << "<p>Unsupported HTTP version</p>";
    ss << "</body></html>\r\n";
    std::string body = ss.str();

    internal_error(std::move(client_sock), body);
}

void Server::method_not_supported(std::unique_ptr<TCPSocket> client_sock, std::string &m)
{
    std::stringstream ss;
    ss << "<html><title>501 Not implemented</title>";
    ss << "<body>\r\n" << " Not Implemented\r\n";
    ss << "<p>Does not implement this method: " << m << "\r\n";
    ss << "<hr><em>HTTP Web server</em>\r\n";
    ss << "</body></html>\r\n";
    std::string body = ss.str();

    internal_error(std::move(client_sock), body);
}

} // namespace simple_http_serve
