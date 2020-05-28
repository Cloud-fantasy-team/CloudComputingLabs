#include <string>
#include <signal.h>
#include <iostream>
#include <condition_variable>
#include "coordinator.hpp"
#include "participant.hpp"
#include "configuration.hpp"
using simple_kv_store::coordinator;
using simple_kv_store::participant;
using simple_kv_store::coordinator_configuration;
using simple_kv_store::participant_configuration;

// std::mutex m;
// std::condition_variable cond;

// void exit_server(int)
// {
//     std::cout << "exitting server\n";
//     cond.notify_all();
// }

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << "[coordinator|participant]" << std::endl;
        return -1;
    }

    if (std::string{argv[1]} == "coordinator")
    {
        coordinator c{coordinator_configuration{}};
        c.start();
    }
    else
    {
        // signal(SIGINT, exit_server);
        participant p{participant_configuration{}};
        p.start();

        std::cout << "server listening at localhost 8001" << std::endl;

        for (;;) {}
        // std::unique_lock<std::mutex> lock(m);
        // cond.wait(lock);
    }
}