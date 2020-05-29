#include <string>
#include <signal.h>
#include <iostream>
#include <condition_variable>
#include "coordinator.hpp"
#include "participant.hpp"
#include "configuration.hpp"
using cdb::coordinator;
using cdb::participant;
using cdb::coordinator_configuration;
using cdb::participant_configuration;

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << "[coordinator|participant]" << std::endl;
        return -1;
    }

    if (std::string{argv[1]} == "coordinator")
    {
        coordinator_configuration conf;
        conf.participant_addrs.push_back("127.0.0.1");
        conf.participant_ports.push_back(8002);
        coordinator c{std::move(conf)};
        c.start();
    }
    else
    {
        participant p{participant_configuration{}};
        p.start();
        std::cout << "server listening at localhost 8001" << std::endl;

        // participant_configuration conf;
        // conf.port = 8002;
        // conf.storage_path = "/tmp/testdb2";
        // participant p2{std::move(conf)};
        // p2.start();
        // std::cout << "server listening at localhost 8002" << std::endl;

        for (;;) {}
    }
}