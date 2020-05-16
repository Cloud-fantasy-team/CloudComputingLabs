/// message.h
/// contains pure message holder.
/// Copyright 2020 Cloud-fantasy team

#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <memory>
#include <cassert>
#include <unordered_map>
#include <string>

namespace simple_http_server
{

static const std::string LINE_END = "\r\n";

/// K/V pairs HTTP headers.
class Headers : public std::unordered_map<std::string, std::string>
{
public:
    std::string serialize() const;
};

/// Base HTTP message.
struct Message
{
    std::string version;
    std::string method;
    std::unique_ptr<Headers> headers;
    std::vector<char> body;

    virtual ~Message() = default;
    virtual std::string serialize() const = 0;
};

/// HTTP request message.
struct Request : public Message
{
    std::string resource;

    Request();
    virtual std::string serialize() const override;
};

/// HTTP response message.
struct Response : public Message
{
    int status_code;
    std::string status;

    Response();
    virtual std::string serialize() const override;

    /// Common status code.
    static const int OK = 200;
    static const int CREATED = 201;
    static const int ACCEPTED = 202;
    static const int NO_CONTENT = 203;
    static const int BAD_REQUEST = 400;
    static const int FORBIDDEN = 403;
    static const int NOT_FOUND = 404;
    static const int REQUEST_TIMEOUT = 408;
    static const int INTERNAL_SERVER_ERROR = 500;
    static const int BAD_GATEWAY = 502;
    static const int SERVICE_UNAVALABLE = 503;
};

} // namespace simple_http_server


#endif