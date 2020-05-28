#include "configuration.hpp"

namespace cdb {

/*
CTORS with default value.
*/
participant_configuration::participant_configuration()
    : configuration(PARTICIPANT)
    , addr("127.0.0.1")
    , port(8001)
    , coordinator_addr("127.0.0.1")
    , coordinator_port(8080)
    , num_worker(2)
    , storage_path("/tmp/testdb") {}

participant_configuration::participant_configuration(participant_configuration && conf)
    : configuration(PARTICIPANT)
    , addr(std::move(conf.addr))
    , port(conf.port)
    , coordinator_addr(std::move(conf.coordinator_addr))
    , coordinator_port(std::move(conf.coordinator_port))
    , num_worker(conf.num_worker)
    , storage_path(std::move(conf.storage_path)) {}

coordinator_configuration::coordinator_configuration()
    : configuration(COORDINATOR)
    , addr("127.0.0.1")
    , port(8080)
    , participant_addrs({"127.0.0.1"})
    , participant_ports({8001}) {}

coordinator_configuration::coordinator_configuration(coordinator_configuration &&conf)
    : configuration(COORDINATOR)
    , addr(std::move(conf.addr))
    , port(conf.port)
    , participant_addrs(std::move(conf.participant_addrs))
    , participant_ports(std::move(conf.participant_ports)) {}

}   // namespace cdb