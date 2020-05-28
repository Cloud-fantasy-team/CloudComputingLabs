#ifndef COORDINATOR_HPP
#define COORDINATOR_HPP

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <map>
#include "command.hpp"
#include "configuration.hpp"
#include "rpc/client.h"
#include "tcp_server/tcp_server.hpp"

namespace simple_kv_store {

using tcp_server_lib::tcp_server;
using tcp_server_lib::tcp_client;

/// Coordinator class.
class coordinator {
public:
    /// Ctors.
    coordinator(coordinator_configuration &&conf);

    /// Starting the server.
    void start();

private:
    /// Tries to connect to all dbs.
    void connect_dbs();

    /// Called by callback workers of [svr] whenever a new
    /// client connection arrives.
    void handle_new_client(std::shared_ptr<tcp_client> client);

    /// Called by callback workers of [svr] whenever a client
    /// sends a new db request.
    /// [prev_data] is not null if previous cmd is not complete yet.
    void handle_db_requests(std::shared_ptr<tcp_client> client, 
                            std::shared_ptr<std::vector<char>> prev_data,
                            tcp_client::read_result &req);

    void handle_db_get_request(std::shared_ptr<tcp_client> client, 
                               get_command cmd);

    void handle_db_set_request(std::shared_ptr<tcp_client> client, 
                               set_command cmd);

    void handle_db_del_request(std::shared_ptr<tcp_client> client, 
                               del_command cmd);

    void commit_db_request(std::shared_ptr<tcp_client> client, std::size_t id, bool &participant_dead);
    void abort_db_request(std::shared_ptr<tcp_client> client, std::size_t id, bool &participant_dead);

    /// Helper.
    void parse_db_requests(std::vector<char> &data, std::vector<std::unique_ptr<command> > &ret, std::size_t &bytes_parsed);

    /// Command errors handler.
    void handle_command_error(std::shared_ptr<tcp_client> client, 
                              std::shared_ptr<std::vector<char>> data,
                              std::runtime_error *e);

    /// Sends a result back to client.
    void send_result(std::shared_ptr<tcp_client> client, std::string const &ret);

    /// Sends an error msg.
    void send_error(std::shared_ptr<tcp_client> client);

private:
    /// config.
    coordinator_configuration conf_;

    /// Underlying TCP server.
    tcp_server svr_;

    /// Connections to participants.
    std::map<std::string/* IP */, std::unique_ptr<rpc::client>> participants_;

    std::atomic<std::size_t> next_id_ = ATOMIC_VAR_INIT(0);

    /// Safety.
    std::mutex participants_mutex_;
    std::condition_variable participants_cond_;
};

} // namespace simple_kv_store


#endif