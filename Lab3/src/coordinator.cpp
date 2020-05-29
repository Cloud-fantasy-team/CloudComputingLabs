#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include "errors.hpp"
#include "command_parser.hpp"
#include "coordinator.hpp"
#include "participant.hpp"

namespace cdb {

coordinator::coordinator(coordinator_configuration &&conf)
    : conf_(std::move(conf))
    , svr_()
    , participants_() {}

void coordinator::start()
{
    svr_.start(conf_.addr, 
               conf_.port,
               std::bind(&coordinator::handle_new_client, this, std::placeholders::_1));

    init_participants();
    heartbeat_participants();
}

/// No locking is needed cause we'll only do it once.
void coordinator::init_participants()
{
    /// Rely on the good old rand and srand to set initial_next_id.
    std::srand(std::time(nullptr));
    int initial_id = std::rand();
    next_id_.store(initial_id);

    /// Create participant clients.
    for (std::size_t i = 0; i < conf_.participant_addrs.size(); i++)
    {
        auto &ip = conf_.participant_addrs[i];
        auto port = conf_.participant_ports[i];
        auto addr = ip + ":" + std::to_string(port);
        try
        {
            participants_[addr] = std::unique_ptr<rpc::client>{ new rpc::client{ip, port} };
            participants_[addr]->set_timeout(200);
            
            participants_[addr]->call("initial_next_id", initial_id).as<bool>();
            /// TODO: If initial_next_id failed, it's probably the coordinator
            /// has been restarted. We'll need to implement a recovery mechanism.
            std::cout << "init participant " << addr << ":" << port;
        }
        catch (std::exception &err)
        {
            /// TODO: LOG.
            std::cout << "remove participant\n";
            participants_.erase(addr);
        }
    }
}

void coordinator::heartbeat_participants()
{
    for (;;)
    {
        const auto &addrs = conf_.participant_addrs;
        const auto &ports = conf_.participant_ports;

        for (std::size_t i = 0; i < addrs.size(); i++)
        {
            try
            {
                rpc::client client{addrs[i], ports[i]};
                client.set_timeout(200);
                client.call("heartbeat");

                std::unique_lock<std::mutex> lock(participants_mutex_);

                std::string addr = addrs[i] + ":" + std::to_string(ports[i]);
                if (!participants_.count(addr))
                {
                    /// We only add it back if we're done.
                    std::cout << "going to recover participant at " << addr << std::endl;
                    if (recover_participant(client))
                    {
                        participants_[addr] = std::unique_ptr<rpc::client>{ new rpc::client{addrs[i], ports[i]} };
                        if (participants_.size() == conf_.participant_addrs.size())
                            del_keys_.clear();
                    }
                }

            }
            catch (std::exception &e) {
                std::cout << "heartbeat failed\n";
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

bool coordinator::recover_participant(rpc::client &client)
{
    std::cout << "recover_participant\n";
    // Lock is already acquired.
    while (!participants_.empty())
    {
        std::vector<char> snapshot;
        bool snapshot_ok = false;
        try
        {
            participants_.begin()->second->set_timeout(300);
            snapshot = std::move(participants_.begin()->second->call("get_snapshot").as<std::vector<char>>());
            snapshot_ok = true;
        }
        catch(const std::exception& e)
        {
            participants_.erase(participants_.begin());
            snapshot_ok = false;
        }

        if (snapshot_ok)
        {
            std::cout << "snapshot OK\n";
            try
            {
                std::size_t id = next_id_;
                client.set_timeout(300);
                client.call("recover", snapshot, del_keys_);
                client.call("initial_next_id", id);
                std::cout << "recover done\n";
                return true;
            }
            catch (std::exception &e)
            {
                std::cout << "recover failed: " << e.what() << std::endl;
                return false;
            }
        }
    }
    return false;
}

void coordinator::handle_new_client(std::shared_ptr<tcp_client> client)
{
    std::cout << "handle_new_client" << std::endl;
    try {
        /// Asynchronously read a db request from client.
        client->async_read({
            1024,   /* 1024 is big enough for most db request. */
            std::bind(&coordinator::handle_db_requests, this, client, nullptr, std::placeholders::_1)
        });
    } catch(std::runtime_error &e) {
        /// Client disconnected.
        /// TODO: LOG
    }
}

void coordinator::handle_db_requests(std::shared_ptr<tcp_client> client, 
                                     std::shared_ptr<std::vector<char>> prev_data,
                                     tcp_client::read_result &req)
{
    if (!req.success)
    {
        /// Does not throw.
        client->disconnect(false);
        return;
    }

    std::cout << "handle_db_requests with " << req.data.size() << " bytes" << std::endl;
    std::vector<char> data;
    if (prev_data)
        data = std::move(*prev_data);
    data.insert(data.end(), req.data.begin(), req.data.end());

    std::vector<std::unique_ptr<command> > cmds;
    bool parse_error = false;
    bool is_incomplete = false;
    std::size_t bytes_parsed = 0;

    /// Try parsing a client command.
    try
    {
        std::vector<char> data_copy{data};
        parse_db_requests(data_copy, cmds, bytes_parsed);
    }
    catch (parse_incomplete_error &e)
    {
        is_incomplete = true;
        parse_error = true;
        std::cout << "parse error: " << e.what() << std::endl;
    }
    catch (std::runtime_error &e)
    {
        parse_error = true;
        std::cout << "parse error: " << e.what() << std::endl;
    }

    /// Dispatch.
    for (const auto &cmd : cmds)
    {
        switch (cmd->type)
        {
        case CMD_GET: {
            /// copy constructed
            get_command get_cmd{ *static_cast<get_command*>(cmd.get()) };
            handle_db_get_request(client, get_cmd);
            break;
        }

        case CMD_SET: {
            /// copy constructed
            set_command set_cmd{ *static_cast<set_command*>(cmd.get()) };
            handle_db_set_request(client, set_cmd);
            break;
        }

        case CMD_DEL: {
            /// copy constructed
            del_command del_cmd{ *static_cast<del_command*>(cmd.get()) };
            handle_db_del_request(client, del_cmd);
            break;
        }

        default:
            send_error(client);
            break;
        }
    }

    if (parse_error)
    {
        /// If this is an parse_incomplete_error, handle_command_error
        /// will re-call handle_db_requests with the previous left bytes.
        handle_command_error(client, 
                             /* Left bytes. */
                             std::shared_ptr<std::vector<char>>{ new std::vector<char>{data.begin() + bytes_parsed, data.end()} },
                             is_incomplete);
        return;
    }

    /// Recurse on itself.
    try
    {
        client->async_read({
            1024,
            std::bind(&coordinator::handle_db_requests, this, client, nullptr, std::placeholders::_1)
        });
    } 
    catch (std::runtime_error &e)
    {
        /// TODO: LOG
        client->disconnect(false);
    }
}

void coordinator::handle_db_get_request(std::shared_ptr<tcp_client> client, 
                                        get_command cmd)
{
    db_get_request req{0, std::move(cmd)};

    {
        /// Acquire lock.
        std::unique_lock<std::mutex> lock(participants_mutex_);

        if (participants_.empty())
        {
            /// The system cannot function.
            send_error(client);
            client->disconnect(false);
            return;
        }

        /// GET will be served directly.
        while (!participants_.empty())
        {
            try {
                auto &p = participants_.begin()->second;
                p->set_timeout(300);

                auto value = p->call("get", req).as<std::string>();
                send_result(client, value);
                return;
            } catch (std::exception &) {
                // Remove this client.
                /// TODO: LOG
                participants_.erase(participants_.begin());
                std::cout << "handle_db_get_request remove participant" << std::endl;
            }
        }
    }

    /// If we've exhausted all dbs. The system is down.
    send_error(client);
}

void coordinator::handle_db_set_request(std::shared_ptr<tcp_client> client, 
                                        set_command cmd)
{
    db_set_request req{next_id_.fetch_add(1), std::move(cmd)};

    /// Acquire lock.
    std::unique_lock<std::mutex> lock(participants_mutex_);
    if (participants_.empty())
    {
        std::cout << "participant empty" << std::endl;
        /// The system cannot function.
        send_error(client);
        client->disconnect(false);
        return;
    }

    /// PREPARE
    bool prepare_ok = true;
    bool participant_dead = false;
    std::cout << "participants_.size() " << participants_.size() << std::endl;
    for (auto iter = participants_.begin(); iter != participants_.end(); )
    {
        try
        {
            std::cout << "prepare_set\n";
            iter->second->set_timeout(300);
            prepare_ok = iter->second->call("prepare_set", req).as<bool>();
            if (!prepare_ok)
                break;
            std::cout << "prepare_set ok\n";
        } 
        catch (std::exception &e)
        {
            /// Unreachable db
            iter = participants_.erase(iter);
            participant_dead = true;
            std::cout << "handle_db_set_request remove participant" << std::endl;
            continue;
        }
        iter++;
    }

    if (participants_.empty())
    {
        send_error(client);
        client->disconnect(false);
        return;
    }

    /// COMMIT
    std::string ret;
    if (prepare_ok)
        commit_db_request(client, req.req_id, participant_dead);

    /// ABORT
    else
        abort_db_request(client, req.req_id, participant_dead);

    if (participant_dead)
        participants_cond_.notify_all();
}

void coordinator::handle_db_del_request(std::shared_ptr<tcp_client> client, 
                                        del_command cmd)
{
    db_del_request req{next_id_.fetch_add(1), std::move(cmd)};

    /// Acquire lock.
    std::unique_lock<std::mutex> lock(participants_mutex_);
    if (participants_.empty())
    {
        std::cout << "participant empty" << std::endl;
        /// The system cannot function.
        send_error(client);
        client->disconnect(false);
        return;
    }

    /// PREPARE
    bool prepare_ok = true;
    bool participant_dead = false;
    for (auto iter = participants_.begin(); iter != participants_.end(); )
    {
        try
        {
            iter->second->set_timeout(300);
            prepare_ok = iter->second->call("prepare_del", req).as<bool>();
            if (!prepare_ok)
                break;
        } 
        catch (std::exception &e)
        {
            /// Unreachable db
            iter = participants_.erase(iter);
            participant_dead = true;
            std::cout << "handle_db_del_request removed participant" << std::endl;
            continue;
        }
        iter++;
    }

    if (participants_.empty())
    {
        send_error(client);
        client->disconnect(false);
        return;
    }

    /// COMMIT
    if (prepare_ok)
    {
        commit_db_request(client, req.req_id, participant_dead);
        
        /// Record DEL cmd that'll be used to recover dead participants.
        if (participants_.size() < conf_.participant_addrs.size())
        {
            auto keys = req.cmd.args();
            del_keys_.insert(keys.begin(), keys.end());
        }
    }
    /// ABORT
    else
        abort_db_request(client, req.req_id, participant_dead);
    
    if (participant_dead)
        participants_cond_.notify_all();
}

void coordinator::commit_db_request(std::shared_ptr<tcp_client> client, 
                                    std::size_t id, 
                                    bool &participant_dead) {
    std::string ret;

    /// Lock has required by caller.
    for (auto iter = participants_.begin(); iter != participants_.end(); )
    {
        try
        {
            std::cout << "before commit" << std::endl;
            iter->second->set_timeout(300);
            ret = iter->second->call("commit", id).as<std::string>();
            std::cout << "commit result: " << ret << std::endl;
        }
        catch (std::exception &e)
        {
            /// Unreachable db.
            iter = participants_.erase(iter);
            participant_dead = true;
            std::cout << "commit_db_request remove participants" << std::endl;
            continue;
        }
        iter++;
    }

    send_result(client, ret);
}

void coordinator::abort_db_request(std::shared_ptr<tcp_client> client, 
                                   std::size_t id, 
                                   bool &participant_dead) 
{
    /// Lock has required by caller.
    for (auto iter = participants_.begin(); iter != participants_.end(); )
    {
        try
        {
            iter->second->set_timeout(300);
            /// If abort rpc returns false, basically it's malfunctioning.
            if (!iter->second->call("abort", id).as<bool>())
                throw std::exception();
        }
        catch (std::exception &e)
        {
            /// Unreachable db.
            iter = participants_.erase(iter);
            participant_dead = true;
            std::cout << "abort_db_request removed participant" << std::endl;
            continue;
        }
        iter++;
    }
    send_error(client);
}

void
coordinator::parse_db_requests(std::vector<char> &data, 
                               std::vector<std::unique_ptr<command> > &ret, 
                               std::size_t &bytes_parsed)
{
    command_parser parser{std::move(data)};

    while (!parser.is_done())
    {
        auto cmd = parser.read_command();
        bytes_parsed = parser.bytes_parsed();
        ret.push_back(std::move(cmd));
    }
}

void coordinator::handle_command_error(std::shared_ptr<tcp_client> client,
                                       std::shared_ptr<std::vector<char>> data,
                                       bool is_incomplete)
{
    if (!is_incomplete)
    {
        send_error(client);
        return;
    }
    std::cout << "handle incomplete error" << std::endl;

    /// Error caused by an incomplete command.
    try
    {
        /// Try again.
        client->async_read({
            1024,
            std::bind(&coordinator::handle_db_requests, this, client, data, std::placeholders::_1)
        });
    } 
    catch(std::runtime_error &e)
    {
        /// TODO: LOG
        client->disconnect(false);
    }
}

void coordinator::send_error(std::shared_ptr<tcp_client> client)
{
    send_result(client, participant::error_string);
}

void coordinator::send_result(std::shared_ptr<tcp_client> client, std::string const &msg)
{
    try {
        client->async_write({
            std::vector<char>{msg.begin(), msg.end()},
            nullptr
        });
    } catch(std::runtime_error &e) {
        /// TODO: LOG
        client->disconnect(false);
    }
}

} // namespace cdb
