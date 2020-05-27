#include <signal.h>
#include <iostream>
#include <condition_variable>
#include "participant.hpp"
#include "configuration.hpp"
using simple_kv_store::participant;
using simple_kv_store::participant_configuration;

std::mutex m;
std::condition_variable cond;

void exit_server(int)
{
    std::cout << "exitting server\n";
    cond.notify_all();
}

int main()
{
    participant p{std::unique_ptr<participant_configuration>{new participant_configuration{}}};
    p.start();

    std::cout << "server listening at localhost 8001" << std::endl;
    signal(SIGINT, exit_server);
    std::unique_lock<std::mutex> lock(m);
    cond.wait(lock);
}