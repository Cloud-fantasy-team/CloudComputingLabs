#ifndef PARSE_ERROR_HPP
#define PARSE_ERROR_HPP

#include <string>
#include <stdexcept>

#define __PARSER_THROW(msg)                                                    \
    {                                                                          \
        throw simple_kv_store::parse_error((msg), __FILE__, __LINE__);         \
    }


namespace simple_kv_store
{

class parse_error : public std::runtime_error
{
public:
    parse_error(const std::string msg, const std::string &file, std::size_t line);

public:
    const std::string &file() const;
    std::size_t line() const;

private:
    std::string file_;
    std::size_t line_;
};

} // namespace simple_kv_store


#endif