/// reporter.cc
/// Copyright 2020 Cloud-fantasy team

#include <ctime>
#include <chrono>
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
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);

    switch (severity)
    {
    case ReportSeverityERROR:   return err_ << std::ctime(&now_time) << "Error ";
    case ReportSeverityINFO:    return info_ << std::ctime(&now_time) << "Info ";
    case ReportSeverityWARN:    return warn_  << std::ctime(&now_time) << "Warn ";
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
