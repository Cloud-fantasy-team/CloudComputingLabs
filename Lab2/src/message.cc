#include <vector>
#include <sstream>

#include "reporter.h"
#include "message.h"

namespace simple_http_server
{

std::string Headers::serialize() const
{
    std::string str;

    for (auto &KV : *this)
        str += KV.first + ": " + KV.second + LINE_END;
    return str;
}

Request::Request()
{
    version = "HTTP/1.1";
    method = "";
    headers.reset(new Headers());
    resource = "";
    body = {};
}

std::string Request::serialize() const
{
    std::stringstream str_stream;
    // Request line.
    str_stream << method << " " << resource  << version << LINE_END;

    // Headers.
    str_stream << headers->serialize();
    str_stream << LINE_END;

    // Body.
    std::string data(body.begin(), body.end());
    str_stream << data << LINE_END;
    return str_stream.str();
}

Response::Response()
{
    version = "HTTP/1.1";
    method = "";
    headers.reset(new Headers());
    status_code = 0;
    status = "";
    body = {};
}

std::string Response::serialize() const
{
    std::stringstream str_stream;
    // Response line.
    str_stream << version << " " << std::to_string(status_code) << " " << status << LINE_END;

    // Headers.
    str_stream << headers->serialize() << LINE_END;

    // Body.
    std::string data(body.begin(), body.end());
    str_stream << data << LINE_END;
    return str_stream.str();
}

}