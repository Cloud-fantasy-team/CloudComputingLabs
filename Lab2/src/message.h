/// message.h
/// Copyright 2020 Cloud-fantasy team

#ifndef MESSAGE_H
#define MESSAGE_H

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
    /// Creator
    static std::unique_ptr<Headers> deserialize(std::string const &header_str);
};

/// Base HTTP message.
class Message
{
private:
    std::string version_;
    std::string method_;
    std::unique_ptr<Headers> headers_;
    std::vector<char> body_;

public:
    virtual std::string serialize() const = 0;
    Message() {}

    /* getters and setters. */
    const std::string &version() const { return version_; }
    std::string &version() { return version_; }
    const std::string &method() const { return method_; }
    std::string &method() { return method_; }
    Headers *headers() const { return headers_.get(); }
    const std::vector<char> &body() const { return body_; }
    std::vector<char> &body() { return body_; }
};

/// HTTP request message.
class Request : public Message
{
private:
    std::string resource_;

public:
    /* getters and setters. */
    const std::string &resource() const { return resource_; }
    std::string &resource() { return resource_; }

    std::string serialize() const override;
    /// Creator.
    static std::unique_ptr<Request> deserialize(std::string const &request_str);
    static bool deserialize_request_line(std::string &str, 
                                        std::string &method, 
                                        std::string &resource, 
                                        std::string &version);
};

/// HTTP response message.
class Response : Message
{
public:
    int status_code_;
    std::string status_;

    /* getters and setters. */
    std::string serialize() const override;
    int status_code() const { return status_code_; }
    std::string &status() { return status_; }
    const std::string &status() const { return status_; }

    /// Creator.
    static std::unique_ptr<Response> deserialize(std::string const &response_str);

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