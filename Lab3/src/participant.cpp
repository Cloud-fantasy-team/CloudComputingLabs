#include <chrono>
#include <unordered_map>
#include "errors.hpp"
#include "participant.hpp"

namespace simple_kv_store {

/// pimpl
/// Append the request to participents pending requests list.
struct participant::prepare_set_t {
    prepare_set_t(participant &p)
        : p_(p) {}

    bool operator()(db_set_request req)
    {
        try {
            std::lock_guard<std::mutex> lock(p_.db_request_mutex_);
            db_request generic_req{std::move(req)};
            p_.db_requests_.insert(std::move(generic_req));

            return true;
        } catch (std::exception &e) {
            // LOG.
            return false;
        }
    }

    participant &p_;
};

/// pimpl
/// FIXME: limitation of rpclib.
/// Append the request to participents pending requests list.
struct participant::prepare_del_t {
    prepare_del_t(participant &p)
        : p_(p) {}

    bool operator()(db_del_request req)
    {
        try {
            std::lock_guard<std::mutex> lock(p_.db_request_mutex_);
            db_request generic_req{std::move(req)};
            p_.db_requests_.insert(std::move(generic_req));

            return true;
        } catch (std::exception &e) {
            // LOG.
            return false;
        }
    }
    participant &p_;
};

/// pimpl
/// Commit update request.
struct participant::commit_handler_t {
    commit_handler_t(participant &p)
        : p_(p)
    {
        /// Bind to handlers.
        dispatchers[CMD_SET] = std::bind(&commit_handler_t::handle_set, this, std::placeholders::_1);
        dispatchers[CMD_DEL] = std::bind(&commit_handler_t::handle_del, this, std::placeholders::_1);
    }

    /// Commits a request with [id] as req_id.
    std::string operator()(std::size_t id)
    {
        std::unique_lock<std::mutex> lock(p_.db_request_mutex_);

        // We have reached to an inconsistent state.
        if (p_.db_requests_.empty())
            __SERVER_THROW("inconsistent state");

        /// Wait until all requests before this requests to be either
        /// committed or aborted.
        /// NOTE: we'll wait for 10 seconds.
        if (p_.next_id_ != -1 && p_.next_id_ < id)
            p_.db_request_cond_.wait_for(lock, std::chrono::seconds{10}, [&]{ return p_.next_id_ == id; });

        // Handle normal cases.
        if (p_.next_id_ == id || p_.next_id_ == -1)
        {
            auto iter = p_.db_requests_.begin();
            // We assume coordinator never fails.
            if (iter->req_id != p_.next_id_)
                __SERVER_THROW("inconsistent state");

            // Dispatching.
            auto handler = dispatchers[iter->cmd->type];
            if (!handler)
                __SERVER_THROW("unrecognizable command");

            std::string ret = handler(iter->cmd.get());
            
            /// Update bookkeeping info.
            p_.db_requests_.erase(p_.db_requests_.begin());
            p_.next_id_ = p_.db_requests_.begin()->req_id;

            return ret;
        }
        else
            /// Spurious wakeup.
            __SERVER_THROW("inconsistent state");
    }

    /// Handle SET key value command.
    std::string handle_set(command *cmd)
    {
        /// Lock is already acquired.

        // Internal server error.
        if (!cmd)
            __SERVER_THROW("cmd is nullptr");

        std::vector<std::string> args = cmd->args();
        std::string &key = args[0];
        std::string &value = args[1];

        /// Deliver to leveldb.
        auto status = p_.db_->Put(leveldb::WriteOptions(), key, value);
        if (status.ok())
            return "+OK\r\n";
        else
            return "-ERROR\r\n";
    }

    /// Handle DEL key1 key2 key3 key4 ... cmd.
    std::string handle_del(command *cmd)
    {
        /// Lock is already acquired.

        if (!cmd)
            __SERVER_THROW("cmd is nullptr");

        std::size_t count = 0;
        std::vector<std::string> keys = cmd->args();
        for (std::string &key : keys)
        {
            auto status = p_.db_->Delete(leveldb::WriteOptions(), key);
            if (status.ok())    count++;
        }

        if (count == 0)
            return "-ERROR\r\n";

        // RESP integer, representing the number of elems deleted.
        std::string ret = ":" + std::to_string(count) + "\r\n";
        return ret;
    }

    /// Dispatcher
    std::unordered_map<
        uint8_t/* command_type */, 
        std::function<std::string(command*)>/* handler */> dispatchers;
    participant &p_;
};

/// pimpl
struct participant::abort_handler_t {
    abort_handler_t(participant &p)
        : p_(p) {}

    /// Aborts a request with [id] as req_id.
    bool operator()(std::size_t id)
    {
        std::unique_lock<std::mutex> lock(p_.db_request_mutex_);

        /// Same as commit.
        if (p_.next_id_ != -1 && p_.next_id_ != id)
            p_.db_request_cond_.wait_for(lock, std::chrono::seconds{10}, [&]() { return p_.next_id_ == id; });

        if (p_.next_id_ == -1 || p_.next_id_ == id)
        {
            /// Upate bookkeeping info.
            p_.db_requests_.erase(p_.db_requests_.begin());
            p_.next_id_ = p_.db_requests_.begin()->req_id;

            return true;
        }
        else
            /// Inconsistent state.
            __SERVER_THROW("inconsistent state");
    }

    participant &p_;
};

participant::participant(const std::string &ip, 
                         uint16_t port, 
                         const std::string & storage_path)
    : svr_(ip, port)
    , prepare_set_(new participant::prepare_set_t(*this))
    , prepare_del_(new participant::prepare_del_t(*this))
    , commit_handler_(new participant::commit_handler_t(*this))
    , abort_handler_(new participant::abort_handler_t(*this))
    , db_(nullptr)
{
    leveldb::Options options;
    options.create_if_missing = true;
    auto status = leveldb::DB::Open(options, storage_path, &db_);

    /// TODO: fix this.
    if (!status.ok())
        __SERVER_THROW("unable to open storage");

    /// Bind 2PC functionalities.
    /// NOTE: the handler throws server_error and crashes if it reaches an inconsistent
    /// state, which is usually caused by failure of coordinator.
    svr_.bind("prepare_set", *prepare_set_);
    svr_.bind("prepare_del", *prepare_del_);
    svr_.bind("commit", *commit_handler_);
    svr_.bind("abort", *abort_handler_);
}

participant::~participant()
{
    delete db_;
    delete prepare_set_;
    delete prepare_del_;
    delete commit_handler_;
    delete abort_handler_;
}

void participant::start()
{
    svr_.run();
}

}    // namespace simple_kv_store