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

}