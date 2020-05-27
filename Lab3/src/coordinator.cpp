#include <chrono>
#include "errors.hpp"
#include "command_parser.hpp"
#include "coordinator.hpp"
#include "participant.hpp"

namespace simple_kv_store {

coordinator::coordinator(std::unique_ptr<coordinator_configuration> conf)
    : conf_(std::move(conf))
    , svr_()
    , participants_() {}

void coordinator::start()
{
    svr_.start(conf_->addr, 
               conf_->port,
               std::bind(&coordinator::handle_new_client, this, std::placeholders::_1));

    /// Create participant clients.
    for (std::size_t i = 0; i < conf_->participant_addrs.size(); i++)
    {
        auto &addr = conf_->participant_addrs[i];
        auto port = conf_->participant_ports[i];
        participants_[addr] = std::unique_ptr<rpc::client>{ new rpc::client{addr, port} };
    }

    connect_dbs();
}

void coordinator::connect_dbs()
{
    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(participants_mutex_);
            if (participants_.size() == conf_->participant_addrs.size())
                participants_cond_.wait(lock, [&]() { return participants_.size() < conf_->participant_addrs.size(); });
        }
    }
}

void coordinator::handle_new_client(std::shared_ptr<tcp_client> client)
{
    try {
        /// Asynchronously read a db request from client.
        client->async_read({
            1024,   /* 1024 is big enough for most db request. */
            std::bind(&coordinator::handle_db_requests, this, client, std::placeholders::_1)
        });
    } catch(std::runtime_error &e) {
        /// Client disconnected.
        /// TODO: LOG
    }
}

void coordinator::handle_db_requests(std::shared_ptr<tcp_client> client, 
                                     tcp_client::read_result &req)
{
    if (!req.success)
    {
        /// Does not throw.
        client->disconnect(false);
        return;
    }

    std::vector<char> data{req.data.begin(), req.data.end()};
    std::vector<std::unique_ptr<command> > cmds;
    bool parse_error = false;

    /// Try parsing a client command.
    try
    {
        parse_db_requests(data, cmds);
    }
    catch (std::runtime_error &e)
    {
        parse_error = true;
    }

    /// Dispatch.
    for (const auto &cmd : cmds)
    {
        switch (cmd->type)
        {
        case CMD_GET: {
            get_command get_cmd{ *static_cast<get_command*>(cmd.get()) };
            handle_db_get_request(client, get_cmd);
            break;
        }

        case CMD_SET: {
            set_command set_cmd{ *static_cast<set_command*>(cmd.get()) };
            handle_db_set_request(client, set_cmd);
            break;
        }

        case CMD_DEL: {
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
        handle_command_error(client, req.data, nullptr);
}

void coordinator::handle_db_get_request(std::shared_ptr<tcp_client> client, 
                                        get_command cmd)
{
    db_get_request req{0, std::move(cmd)};

    /// Acquire lock.
    std::unique_lock<std::mutex> lock(participants_mutex_);
    if (participants_.empty())
    {
        /// The system cannot function.
        send_error(client);
        return;
    }

    /// GET will be served directly.
    while (!participants_.empty())
    {
        try {
            auto &p = participants_.begin()->second;
            p->set_timeout(200);

            auto value = p->call("get", req).as<std::string>();
            send_result(client, value);
            return;
        } catch (std::exception &) {
            // Remove this client.
            /// TODO: LOG
            participants_.erase(participants_.begin());
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
        /// The system cannot function.
        send_error(client);
        return;
    }

    /// PREPARE
    bool prepare_ok = true;
    bool participant_dead = false;
    for (auto iter = participants_.begin(); iter != participants_.end(); )
    {
        try
        {
            iter->second->set_timeout(200);
            prepare_ok = iter->second->call("prepare_set", req).as<bool>();
            if (!prepare_ok)
                break;
        } 
        catch (std::exception &e)
        {
            /// Unreachable db
            iter = participants_.erase(iter);
            participant_dead = true;
            continue;
        }
        iter++;
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
    db_del_request req{0, std::move(cmd)};

    /// Acquire lock.
    std::unique_lock<std::mutex> lock(participants_mutex_);
    if (participants_.empty())
    {
        /// The system cannot function.
        send_error(client);
        return;
    }

    /// PREPARE
    bool prepare_ok = true;
    bool participant_dead = false;
    for (auto iter = participants_.begin(); iter != participants_.end(); )
    {
        try
        {
            iter->second->set_timeout(200);
            prepare_ok = iter->second->call("prepare_del", req).as<bool>();
            if (!prepare_ok)
                break;
        } 
        catch (std::exception &e)
        {
            /// Unreachable db
            iter = participants_.erase(iter);
            participant_dead = true;
            continue;
        }
        iter++;
    }

    /// COMMIT
    if (prepare_ok)
        commit_db_request(client, req.req_id, participant_dead);
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
            iter->second->set_timeout(200);
            ret = iter->second->call("commit", id).as<std::string>();
        }
        catch (std::exception &e)
        {
            /// Unreachable db.
            iter = participants_.erase(iter);
            participant_dead = true;
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
            iter->second->set_timeout(200);
            /// If abort rpc returns false, basically it's malfunctioning.
            if (!iter->second->call("abort", id).as<bool>())
                throw std::exception();
        }
        catch (std::exception &e)
        {
            /// Unreachable db.
            iter = participants_.erase(iter);
            participant_dead = true;
            continue;
        }
        iter++;
    }
    send_error(client);
}

void
coordinator::parse_db_requests(std::vector<char> &data, std::vector<std::unique_ptr<command> > &ret)
{
    command_parser parser{std::move(data)};

    while (!parser.is_done())
    {
        auto cmd = parser.read_command();
        ret.push_back(std::move(cmd));
    }
}

void coordinator::handle_command_error(std::shared_ptr<tcp_client> client,
                                       std::vector<char> &,
                                       std::runtime_error *)
{
    /// TODO: Handle incomplete message.
    send_error(client);
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

} // namespace simple_kv_store
