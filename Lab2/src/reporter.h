#ifndef REPORTER_H
#define REPORTER_H

#include <fstream>
#include <iostream>
#include <string>

#define report(f)  simple_http_server::Reporter::log((f), __LINE__, __FILE__)

enum ReportSeverity
{
    ERROR,
    INFO,
    WARN
};

namespace simple_http_server
{

/// Setup err, warn, info files.
void initialize_reporter(std::string const &err,
                        std::string const &warn,
                        std::string const &info);

/// Singleton reporter class.
class Reporter
{
private:
    friend void initialize_reporter(std::string const &err,
                        std::string const &warn,
                        std::string const &info);

    static std::ofstream info_;
    static std::ofstream warn_;
    static std::ofstream err_;

    static std::ostream &file(const int severity);

    /// Disallow instantiation.
    Reporter() = delete;
    Reporter(const Reporter&) = delete;
    Reporter(const Reporter&&) = delete;
    void operator=(Reporter&) = delete;
    void operator=(Reporter&&) = delete;

public:
    static std::ostream &log(int severity, int line, std::string const &file);
};

}   // namespace simple_http_server
#endif