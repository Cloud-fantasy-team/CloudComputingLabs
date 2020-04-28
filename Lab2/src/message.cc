#include <vector>
#include <sstream>

#include "reporter.h"
#include "message.h"

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

// Trim trailing carrage return.
static void trim_cr(std::string &s)
{
    if (s.empty())  return;
    if (s[s.length() - 1] == '\r')
        s = s.substr(0, s.length() - 1);
}

// Trim whitespace.
static void trim_ws(std::string &s)
{
    if (s.empty())  return;
    while (s[0] == ' ')
        s = s.substr(1, s.length());

    if (s.empty())  return;
    while (s.back() == ' ')
        s = s.substr(0, s.length() - 1);
}

std::string Headers::serialize() const
{
    std::string str;

    for (auto &KV : *this)
        str += KV.first + ": " + KV.second + LINE_END;
    return str;
}

/// static
std::unique_ptr<Headers> Headers::deserialize(std::string const &str)
{
    auto hdr_obj = std::unique_ptr<Headers>(new Headers());
    auto hdrs = split(str, LINE_END);

    for (auto &hd : hdrs)
    {
        auto kv = split(hd, ": ");

        if (kv.size() != 2)
        {
            report(ERROR) << "invalid header" << std::endl;
            abort();
        }

        (*hdr_obj)[kv[0]] = kv[1];
    }
    return hdr_obj;
}

std::string Request::serialize() const
{
    std::stringstream str_stream;
    // Request line.
    str_stream << method() << " " << resource() << " HTTP/" << version() << LINE_END;

    // Headers.
    str_stream << headers()->serialize();
    str_stream << LINE_END;

    // Body.
    std::string data(body().begin(), body().end());
    str_stream << data << LINE_END;
    return str_stream.str();
}

/// Static
std::unique_ptr<Request> Request::deserialize(std::string const &str)
{
    std::unique_ptr<Request> req(new Request());
    std::istringstream iss(str);
    std::string line;

    // Request line.
    std::getline(iss, line, '\n');
    trim_cr(line);
    std::vector<std::string> v = split(line, " ");
    if (v.size() != 3)
    {
        report(ERROR) << "invalid http request: " << line << std::endl;
        return nullptr;
    }
    req->method() = v[0];
    req->resource() = v[1];
    req->version() = v[2];

    // Headers.
    for (std::getline(iss, line, '\n'); line != "\r"; std::getline(iss, line, '\n'))
    {
        std::vector<std::string> hd = split(line, ":");
        if (hd.size() != 2)
        {
            report(ERROR) << "invalid header: " << line << std::endl;
            continue;
        }

        trim_ws(hd[0]);
        trim_ws(hd[1]);
        req->headers()->insert(std::make_pair(hd[0], hd[1]));
    }

    // Body.
    std::string res = iss.str();
    req->body() = std::vector<char>(res.begin(), res.end());
    return req;
}

std::string Response::serialize() const
{
    std::stringstream str_stream;
    // Response line.
    str_stream << "HTTP/" << version() << " " << std::to_string(status_code_) << " " << status() << LINE_END;

    // Headers.
    str_stream << headers()->serialize() << LINE_END;

    // Body.
    std::string data(body().begin(), body().end());
    str_stream << data << LINE_END;
    return str_stream.str();
}

/// Static
std::unique_ptr<Response> Response::deserialize(std::string const &str)
{
    // TODO
    return nullptr;
}

}