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

int main()
{
    participant_configuration conf;
    conf.port = 8002;
    conf.storage_path = "/tmp/testdb2";
    participant p2{std::move(conf)};
    p2.start();
    std::cout << "server listening at localhost 8002" << std::endl;

    for(;;){}
}