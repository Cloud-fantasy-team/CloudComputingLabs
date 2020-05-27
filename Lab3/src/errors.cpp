#include "errors.hpp"

namespace simple_kv_store {

parse_syntax_error::parse_syntax_error(const std::string msg, const std::string &file, std::size_t line)
    : std::runtime_error(msg)
    , file_(file)
    , line_(line) {}

const std::string &parse_syntax_error::file() const
{
    return file_;
}

std::size_t parse_syntax_error::line() const
{
    return line_;
}

parse_incomplete_error::parse_incomplete_error(const std::string msg, const std::string &file, std::size_t line)
    : std::runtime_error(msg)
    , file_(file)
    , line_(line) {}

const std::string &parse_incomplete_error::file() const
{
    return file_;
}

std::size_t parse_incomplete_error::line() const
{
    return line_;
}

server_error::server_error(const std::string msg, const std::string &file, std::size_t line)
    : std::runtime_error(msg)
    , file_(file)
    , line_(line) {}


const std::string &server_error::file() const
{
    return file_;
}

std::size_t server_error::line() const
{
    return line_;
}
}