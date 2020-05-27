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

namespace simple_kv_store {

/// Store server class. We're utilizing leveldb for real and fast storage.
class participant {
public:
    /// ctors.
    participant(const std::string &ip, 
                uint16_t port, 
                const std::string &storage_path = "/tmp/testdb");

    ~participant();

    void start();

private:
    /// 2PC.
    /// Different types of update RPCs are handled by different objects.
    /// NOTE: we'll only run 2PC on update commands.
    struct prepare_set_t;
    struct prepare_del_t;
    struct commit_handler_t;
    struct abort_handler_t;

    friend prepare_set_t;
    friend prepare_del_t;
    friend commit_handler_t;
    friend abort_handler_t;

private:
    /// Comparator.
    struct db_request_cmp {
        bool 
        operator()(const db_request &lhs, const db_request &rhs) const
        {
            return lhs.req_id < rhs.req_id;
        }
    };

    /// The actual server for responding RPCs.
    rpc::server svr_;

    /// The actual handlers.
    prepare_set_t *prepare_set_;
    prepare_del_t *prepare_del_;
    commit_handler_t *commit_handler_;
    abort_handler_t *abort_handler_;

    /// TODO: change this to an LRU?
    /// Pending requests.
    std::set<db_request, db_request_cmp> db_requests_;
    /// Next request id to be handled.
    std::atomic<ssize_t> next_id_ = ATOMIC_VAR_INIT(-1);

    /// Synchronization.
    std::mutex db_request_mutex_;
    std::condition_variable db_request_cond_;

    /// Real storage.
    leveldb::DB *db_;
};

} // namespace simple_kv_store


#endif