#include <string>
#include <signal.h>
#include <iostream>
#include <condition_variable>
#include "coordinator.hpp"
#include "participant.hpp"
#include "configuration.hpp"
#include "Flags.hh"
using cdb::coordinator;
using cdb::participant;
using cdb::coordinator_configuration;
using cdb::participant_configuration;

/// Helper
static bool parse_addrs(std::string const &addrs, 
                        std::vector<std::string> &ips, 
                        std::vector<std::uint16_t> &ports)
{
    bool ok = true;
    auto trim_ws = [](std::string const &s)
    {
        if (s.empty())  return s;
        int idx = 0;
        while (idx < s.size() && 
               (s[idx] == ' ' || s[idx] == '\t' || s[idx] == '\n' || s[idx] == '\r'))
            idx++;

        int end_idx = s.size() - 1;
        while (end_idx >= 0 &&
               (s[end_idx] == ' ' || s[end_idx] == '\t' || s[end_idx] == '\n' || s[end_idx] == '\r'))
            end_idx--;

        return s.substr(idx, end_idx - idx + 1);
    };

    auto split_addr = [](std::string const &addr, std::string &ip, std::uint16_t &port)
    {
        int idx = 0;
        while (idx < addr.size() && addr[idx] != ':')
            idx++;
        
        if (idx == addr.size())
            return false;

        ip = addr.substr(0, idx);
        port = std::stoi(addr.substr(idx + 1));
        return true;
    };

    std::stringstream ss{addrs};
    std::string addr;
    while (std::getline(ss, addr, ';'))
    {
        std::string ip;
        std::uint16_t port;
        if (!split_addr(addr, ip, port))
            return false;

        ips.push_back(ip);
        ports.push_back(port);
    }

    return ok;
}

int main(int argc, char *argv[])
{
    std::string mode;               /* E.g., "participant" | "coordinator" */
    std::string ip;                 /* E.g., "127.0.0.1" */
    std::uint16_t port;             /* E.g., "8001" */
    std::string config_path;        /* E.g., "./participant.conf" */

    std::string participant_addrs;  /* E.g., "127.0.0.1:8001;127.0.0.1:8002;" */
    std::string coordinator_addr;   /* E.g., "127.0.0.1:8080" */

    bool help;
    Flags flags;
    flags.Bool(help, 'h', "help", "print this message and exit");
    flags.Var(mode, 'm', "mode", std::string{"participant"}, "specify the mode of the server. [\"coordinator\" | \"participant\"]. Defaulted to \"participant\"");
    flags.Var(ip, 'a', "ip", std::string{""}, "specify an ip address. Defaulted to 127.0.0.1");
    flags.Var(config_path, 'c', "config_path", std::string{""}, "specify the path to config");
    flags.Var(port, 'p', "port", std::uint16_t{0}, "specify a port.");
    flags.Var(participant_addrs, 'P', "participant_addrs", std::string{""}, "specify a list of participant addrs separated by ';'. E.g. ip1:port1;ip2:port2");
    flags.Var(coordinator_addr, 'C', "coordinator_addr", std::string{""}, "specify the address of coordinator. E.g., 127.0.0.1:8080");

    /// TODO: parse the configuration.
    if (!flags.Parse(argc, argv))
    {
        flags.PrintHelp(argv[0]);
        abort();
    }

    if (help)
    {
        flags.PrintHelp(argv[0]);
        return 0;
    }

    std::vector<std::string> ip_addrs;
    std::vector<std::uint16_t> ports;

    if (mode == "coordinator")
    {
        coordinator_configuration conf;

        if (!ip.empty())
            conf.addr = ip;
        if (port != 0)
            conf.port = port;

        if (participant_addrs.empty())
        {
            std::cerr << "coordinator was started with no participants" << std::endl;
            flags.PrintHelp(argv[0]);
            abort();
        }

        if (!parse_addrs(participant_addrs, ip_addrs, ports))
        {
            std::cerr << "invalid participant addresses" << std::endl;
            flags.PrintHelp(argv[0]);
            abort();
        }

        conf.participant_addrs = std::move(ip_addrs);
        conf.participant_ports = std::move(ports);

        coordinator c{std::move(conf)};
        std::cout << "coordinator running at [" << conf.addr << ":" << conf.port << "]\n";
        c.start();
    }
    else
    {
        participant_configuration conf;

        if (!ip.empty())
            conf.addr = ip;
        if (port != 0)
            conf.port = port;

        if (coordinator_addr.empty())
        {
            std::cerr << "participant was started with no coordinator" << std::endl;
            flags.PrintHelp(argv[0]);
            abort();
        }

        std::vector<std::string> ip;
        std::vector<std::uint16_t> port;
        if (!parse_addrs(coordinator_addr, ip, port))
        {
            std::cerr << "invalid coordinator address" << std::endl;
            flags.PrintHelp(argv[0]);
            abort();
        }

        conf.coordinator_addr = ip[0];
        conf.coordinator_port = port[0];

        participant p{std::move(conf)};
        std::cout << "participant running at [" << conf.addr << ":" << conf.port << "]\n";
        p.start();
    }
}