/// File configuration.hpp
/// ======================
/// Copyright 2020 Cloud-fantasy team
/// This file contains configuration info needed by both
/// coordinator and participants.
#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP
#include <string>
#include <vector>

namespace cdb {

/// Configuration base class.
struct configuration {
    enum mode_t {
        COORDINATOR,
        PARTICIPANT,
        UNKNOWN
    };

    mode_t mode;
    configuration(mode_t mode) : mode(mode) {}
};

/// Used by coordinator.
struct coordinator_configuration : public configuration {
    coordinator_configuration();
    coordinator_configuration(coordinator_configuration &&);

    /// IP address and port used by the coordinator.
    std::string addr;
    std::uint16_t port;

    /// IP address and ports of participants.
    std::vector<std::string> participant_addrs;
    std::vector<std::uint16_t> participant_ports;
};

/// Used by participants.
struct participant_configuration : public configuration {
    participant_configuration();
    participant_configuration(participant_configuration &&);

    /// Address used by itself.
    std::string addr;
    std::uint16_t port;

    /// Info of coordinator.
    std::string coordinator_addr;
    std::uint16_t coordinator_port;

    /// Number of callback workers.
    std::size_t num_worker;

    /// Path to persistent storage.
    std::string storage_path;
};

} // namespace cdb


#endif