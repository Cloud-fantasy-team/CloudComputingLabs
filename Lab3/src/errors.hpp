#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <string>
#include <stdexcept>

#define __PARSER_THROW_SYNTAX(msg)                                                    \
    {                                                                                 \
        throw simple_kv_store::parse_syntax_error((msg), __FILE__, __LINE__);         \
    }

#define __PARSER_THROW_INCOMPLETE(msg)                                                \
    {                                                                                 \
        throw simple_kv_store::parse_incomplete_error((msg), __FILE__, __LINE__);     \
    }

#define __SERVER_THROW(msg)                                                    \
    {                                                                          \
        throw simple_kv_store::server_error((msg), __FILE__, __LINE__);        \
    }

namespace simple_kv_store
{

/// TODO: Fix this mess.

class parse_incomplete_error : public std::runtime_error
{
public:
    parse_incomplete_error(const std::string msg, const std::string &file, std::size_t line);

public:
    const std::string &file() const;
    std::size_t line() const;

private:
    std::string file_;
    std::size_t line_;
};

class parse_syntax_error : public std::runtime_error
{
public:
    parse_syntax_error(const std::string msg, const std::string &file, std::size_t line);

public:
    const std::string &file() const;
    std::size_t line() const;

private:
    std::string file_;
    std::size_t line_;
};

class server_error : public std::runtime_error
{
public:
    server_error(const std::string msg, const std::string &file, std::size_t line);

public:
    const std::string &file() const;
    std::size_t line() const;

private:
    std::string file_;
    std::size_t line_;
};

} // namespace simple_kv_store


#endif