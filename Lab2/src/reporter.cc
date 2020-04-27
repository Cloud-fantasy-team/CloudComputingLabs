#include <cassert>
#include "reporter.h"

namespace simple_http_server
{

std::ofstream Reporter::err_;
std::ofstream Reporter::warn_;
std::ofstream Reporter::info_;

void initialize_reporter(std::string const &err,
                        std::string const &warn,
                        std::string const &info)
{
    Reporter::err_.open(err);
    Reporter::warn_.open(info);
    Reporter::info_.open(info);
}

std::ostream &Reporter::file(const int severity)
{
    switch (severity)
    {
    case ERROR:   return err_;
    case INFO:    return info_;
    case WARN:    return warn_;
    default:
        assert(0);
    }
}

std::ostream &Reporter::log(int severity, int line, std::string const &filename)
{
    auto &f = file(severity);

    f << filename << " " << line << ": ";
    return f;
}

} // namespace simple_http_server
