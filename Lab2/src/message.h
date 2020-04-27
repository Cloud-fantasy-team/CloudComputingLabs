/// message.h
/// Copyright 2020 Cloud-fantasy team

#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>
#include <unordered_map>
#include <string>

namespace simple_http_server
{

/// NOTE: we're only implementing GET/POST.
enum class Method : uint8_t
{
    GET,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    PATCH
};

static const std::string LINE_END = "\r\n";

/// K/V pairs HTTP headers.
class Headers : public std::unordered_map<std::string, std::string>
{
public:
    std::string serialize() const;
    /// Creator
    static std::unique_ptr<Headers> deserialize(std::string const &header_str);
};

/// Base HTTP message.
class Message
{
protected:
    std::string version_;
    Method method_;
    std::unique_ptr<Headers> headers_;
    void *body_;
    size_t body_len_;

public:
    virtual std::string serialize() const = 0;

    std::string version() const { return version_; }
    Method method() const { return method_; }
    Headers *headers() const { return headers_.get(); }
    void *body() const { return body_; }
    size_t body_len() const { return body_len_; }
};

/// HTTP request message.
class Request : public Message
{
public:
    std::string serialize() const override;
    /// Creator.
    static std::unique_ptr<Request> deserialize(std::string const &request_str);
};

/// HTTP response message.
class Response : Message
{
private:
    int response_code_;

public:
    std::string serialize() const override;
    /// Creator.
    static std::unique_ptr<Request> deserialize(std::string const &response_str);

public:
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