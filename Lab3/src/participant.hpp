/// File participant.hpp
/// ====================
/// Copyright 2020 Cloud-fantasy team
/// This file contains the store server(participant) class.

#ifndef PARTICIPANT_HPP
#define PARTICIPANT_HPP

#include <atomic>
#include <condition_variable>
#include <string>
#include <set>
#include <mutex>
#include "leveldb/db.h"
#include "rpc/server.h"
#include "message.hpp"
#include "configuration.hpp"

namespace cdb {

/// Store server class. We're utilizing leveldb for real and fast storage.
class participant {
public:
    /// ctors.
    participant(participant_configuration &&conf);

    ~participant();

    /// Start coordinator. Blocks current thread.
    void start();

    /// Start coordinator in separate threads.
    void async_start();

    static const std::string error_string;
    static const std::string update_ok_string;

private:
    /// RPC handler types.
    struct get_handler_t;
    struct prepare_set_t;
    struct prepare_del_t;
    struct commit_handler_t;
    struct abort_handler_t;
    struct set_initial_next_id_handler_t;
    struct get_snapshot_t;
    struct recover_t;
    struct heartbeat_t;

    friend get_handler_t;
    friend prepare_set_t;
    friend prepare_del_t;
    friend commit_handler_t;
    friend abort_handler_t;
    friend set_initial_next_id_handler_t;
    friend get_snapshot_t;
    friend recover_t;
    friend heartbeat_t;

private:
    /// Comparator.
    struct db_request_cmp {
        bool 
        operator()(const db_request &lhs, const db_request &rhs) const
        {
            return lhs.req_id < rhs.req_id;
        }
    };

    participant_configuration conf_;

    /// The actual server for responding RPCs.
    rpc::server svr_;

    /// The actual handlers.
    get_handler_t *get_handler_;
    prepare_set_t *prepare_set_;
    prepare_del_t *prepare_del_;
    commit_handler_t *commit_handler_;
    abort_handler_t *abort_handler_;
    set_initial_next_id_handler_t *initial_next_id_;
    heartbeat_t *heartbeat_;
    get_snapshot_t *get_snapshot_;
    recover_t *recover_;

    /// Pending requests.
    std::set<db_request, db_request_cmp> db_requests_;

    /// Monotonically increasing number. We'll use 1 as increment.
    /// The coordinator must assign ids correctly.
    /// The coordinator must rpc "initial_next_id" to set it.
    /// If the coordinator doesn't rpc "initial_next_id", participant will use the
    /// 0 as the initial next_id_.
    std::atomic<ssize_t> next_id_ = ATOMIC_VAR_INIT(0);

    /// Synchronization.
    std::mutex db_request_mutex_;
    std::condition_variable db_request_cond_;

    /// Real storage.
    leveldb::DB *db_;

    /// Flag indicates whether coordinator is started.
    std::atomic<bool> is_started_ = ATOMIC_VAR_INIT(false);
};

} // namespace cdb


#endif