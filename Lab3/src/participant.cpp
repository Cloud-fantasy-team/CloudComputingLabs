#include <iostream>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include "command_parser.hpp"
#include "errors.hpp"
#include "participant.hpp"

namespace cdb {

const std::string participant::error_string = "-ERROR\r\n";
const std::string participant::update_ok_string = "+OK\r\n";

/// pimpl
struct participant::get_handler_t {
    get_handler_t(participant &p)
        : p_(p) {}

    /// Simply return the results.
    std::string operator()(db_get_request req)
    {
        std::string key = req.cmd.key();
        std::string value;
        leveldb::Status status = p_.db_->Get(leveldb::ReadOptions(), key, &value);

        std::cout << "GET " << req.cmd.key() << std::endl;
        if (status.ok())
            return encode_result(value);
        else
            return encode_result("nil");
    }

    /// Encode result in RESP format.
    std::string encode_result(std::string const &value)
    {
        std::stringstream ss(value);
        std::string token;

        std::stringstream ret;
        std::size_t count = 0;
        while (std::getline(ss, token, ' '))
        {
            count++;
            ret << encode_bulk_string(token);
        }
        return "*" + std::to_string(count) + command_parser::separator + ret.str();
    }

    inline std::string encode_bulk_string(std::string const &str)
    {
        std::string bulk{"$" + std::to_string(str.length()) + 
                        command_parser::separator + str + command_parser::separator};
        return bulk;
    }

    participant &p_;
};

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

            std::cout << "PREPARE SET" << std::endl;
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

            std::cout << "PREPARE DEL \n";
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

        std::cout << "COMMIT" << std::endl;
        // We have reached to an inconsistent state.
        if (p_.db_requests_.empty())
            __SERVER_THROW("inconsistent state");

        /// Wait until all requests before this requests to be either
        /// committed or aborted.
        /// The reason that iter->req_id != p_.next_id_ holds is because
        /// the coordinator is capable of sending at most as many as the
        /// number of callback worker threads that the participants have.
        /// NOTE: we'll wait for 200 milli-seconds.
        if (p_.next_id_ < id)
            p_.db_request_cond_.wait_for(lock, std::chrono::microseconds{200}, [&]{ return p_.next_id_ == id; });

        // Handle normal cases.
        if (p_.next_id_ == id)
        {
            auto iter = p_.db_requests_.begin();
            if (iter->req_id != p_.next_id_)
                /// Coordinator's malfunctioning.
                __SERVER_THROW("inconsistent state");

            // Dispatching.
            auto handler = dispatchers[iter->cmd->type];
            if (!handler)
                __SERVER_THROW("unrecognizable command");

            std::string ret = handler(iter->cmd.get());
            
            /// Update bookkeeping info.
            p_.db_requests_.erase(p_.db_requests_.begin());
            p_.next_id_.fetch_add(1);

            p_.db_request_cond_.notify_all();
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
            return participant::update_ok_string;
        else
            return participant::error_string;
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
            leveldb::Status status;
            std::string value;
            bool exists = false;
            status = p_.db_->Get(leveldb::ReadOptions(), key, &value);
            exists = status.ok();

            status = p_.db_->Delete(leveldb::WriteOptions(), key);
            if (status.ok() && exists)    count++;
        }

        if (count == 0)
            return participant::error_string;

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

    /// Aborts a request with [id].
    bool operator()(std::size_t id)
    {
        std::unique_lock<std::mutex> lock(p_.db_request_mutex_);

        std::cout << "ABORT\n";
        /// Same as commit.
        if (p_.next_id_ != id)
            p_.db_request_cond_.wait_for(lock, std::chrono::seconds{10}, [&]() { return p_.next_id_ == id; });

        if (p_.next_id_ == id)
        {
            /// Upate bookkeeping info.
            p_.db_requests_.erase(p_.db_requests_.begin());
            p_.next_id_.fetch_add(1);

            p_.db_request_cond_.notify_all();
            return true;
        }
        else
            /// Inconsistent state.
            __SERVER_THROW("inconsistent state");
    }

    participant &p_;
};

struct participant::set_initial_next_id_handler_t {
    set_initial_next_id_handler_t(participant &p)
        : p_(p) {}

    bool operator()(ssize_t val)
    {
        ssize_t zero = 0;
        return p_.next_id_.compare_exchange_strong(zero, val);
    }

    participant &p_;
};

struct participant::heartbeat_t {
    heartbeat_t(participant &p)
        : p_(p) {}

    void operator()() const {}

    participant &p_;
};


struct participant::get_snapshot_t {
    get_snapshot_t(participant &p)
        : p_(p) {}

    std::vector<char> operator()() const
    {
        std::cout << "GET_SNAPSHOT\n";
        std::vector<char> data;
        auto it = p_.db_->NewIterator(leveldb::ReadOptions());
        
        for (it->SeekToFirst(); it->Valid(); it->Next())
            encode_pair(data, it->key().ToString(), it->value().ToString());

        return data;
    }

    void encode_pair(std::vector<char> &data, 
                    const std::string &key, 
                    const std::string &value) const
    {
        /// 2 bytes to store the key size, in little endian order.
        short key_size = static_cast<short>(key.size());
        data.insert(data.end(), key_size & 0xFF);
        data.insert(data.end(), (key_size >> 8) & 0xFF);
        data.insert(data.end(), key.begin(), key.end());

        /// 2 bytes to store the value size.
        short value_size = static_cast<short>(value.size());
        data.insert(data.end(), value_size & 0xFF);
        data.insert(data.end(), (value_size >> 8) & 0xFF);
        data.insert(data.end(), value.begin(), value.end());
    }

    participant &p_;
};

/// FIXME: The current recovery still ignores any deleted keys.
struct participant::recover_t {
    recover_t(participant &p)
        : p_(p) {}

    bool operator()(std::vector<char> data, std::set<std::string> del_keys)
    {
        std::cout << "RECOVER\n";
        /// First apply the deleted keys.
        for (auto &key : del_keys)
        {
            auto s = p_.db_->Delete(leveldb::WriteOptions(), key);
            if (!s.ok())    return false;
        }

        /// Then update KV pairs.
        for (std::size_t i = 0; i < data.size(); )
        {
            std::vector<std::string> pair;
            for (std::size_t j = 0; j < 2; j++)
            {
                short size;
                char *ptr = reinterpret_cast<char*>(&size);

                if (i + 1 >= data.size())
                {
                    /// TODO: LOG
                    return false;
                }

                *ptr++ = is_big_endian() ? data[i + 1] : data[i];
                *ptr = is_big_endian() ? data[i] : data[i + 1];
                i += 2;

                std::string item{data.begin() + i, data.begin() + i + size};
                i += size;
                pair.push_back(item);
            }

            auto s = p_.db_->Put(leveldb::WriteOptions(), pair[0], pair[1]);
            if (!s.ok())
                return false;
        }
        std::cout << "recovery success\n";
        return true;
    }

    bool is_big_endian(void)
    {
        union {
            uint32_t i;
            char c[4];
        } bint = {0x01020304};

        return bint.c[0] == 1; 
    }


    participant &p_;
};

participant::participant(participant_configuration &&conf)
    : conf_(std::move(conf))
    , svr_(conf_.addr, conf_.port)
    , get_handler_(new participant::get_handler_t(*this))
    , prepare_set_(new participant::prepare_set_t(*this))
    , prepare_del_(new participant::prepare_del_t(*this))
    , commit_handler_(new participant::commit_handler_t(*this))
    , abort_handler_(new participant::abort_handler_t(*this))
    , initial_next_id_(new participant::set_initial_next_id_handler_t(*this))
    , heartbeat_(new participant::heartbeat_t(*this))
    , get_snapshot_(new participant::get_snapshot_t(*this))
    , recover_(new participant::recover_t(*this))
    , db_(nullptr)
{
    leveldb::Options options;
    options.create_if_missing = true;
    auto status = leveldb::DB::Open(options, conf_.storage_path, &db_);

    if (!status.ok())
        __SERVER_THROW("unable to open storage");

    /// Bind 2PC functionalities.
    /// NOTE: the handler throws server_error and crashes if it reaches an inconsistent
    /// state, which is usually caused by failure of coordinator.
    svr_.bind("get", *get_handler_);
    svr_.bind("prepare_set", *prepare_set_);
    svr_.bind("prepare_del", *prepare_del_);
    svr_.bind("commit", *commit_handler_);
    svr_.bind("abort", *abort_handler_);
    svr_.bind("initial_next_id", *initial_next_id_);
    svr_.bind("heartbeat", *heartbeat_);
    svr_.bind("get_snapshot", *get_snapshot_);
    svr_.bind("recover", *recover_);
}

participant::~participant()
{
    delete db_;
    delete prepare_set_;
    delete prepare_del_;
    delete commit_handler_;
    delete abort_handler_;
    delete initial_next_id_;
    delete heartbeat_;
    delete get_snapshot_;
    delete recover_;
}

void participant::start()
{
    svr_.async_run(conf_.num_worker);
}

}    // namespace cdb