/// File message.hpp
/// ================
/// Copyright 2020 Cloud-fantasy team
/// This file contains message types used for
/// manipulating the KV store.
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "command.hpp"
#include "rpc/msgpack.hpp"

namespace simple_kv_store {

/*
The reason to separate coordinator/participant req info
is because of the limitation of serilization library. Or maybe because I've got
no more time to spend on the msgpack docs.
*/

/*
Coordinator. 
*/

/// Used by coordinator.
/// struct db_get_request represents a GET cmd request to the KV store.
struct db_get_request {
    /// NOTE: [req_id] is determined by the coordinator.
    std::size_t req_id;
    get_command cmd;

    db_get_request() = default;
    ~db_get_request() = default;

    /// Convenient ctor.
    db_get_request(std::size_t req_id, get_command &&cmd);

    /// Make it serializable.
    MSGPACK_DEFINE_ARRAY(req_id, cmd)
};

/// Used by the coordinator.
/// struct db_set_request represents a SET cmd request to the KV store.
struct db_set_request {
    /// NOTE: [req_id] is determined by the coordinator.
    std::size_t req_id;
    set_command cmd;

    db_set_request() = default;
    ~db_set_request() = default;

    /// Convenient ctor.
    db_set_request(std::size_t req_id, set_command &&cmd);

    MSGPACK_DEFINE_ARRAY(req_id, cmd);
};

/// Used by the coordinator.
/// struct db_del_request represents a DEL cd request to the KV store.
struct db_del_request {
    /// NOTE: [req_id] is determined by the coordinator.
    std::size_t req_id;
    del_command cmd;

    db_del_request() = default;
    ~db_del_request() = default;

    /// Convenient ctor.
    db_del_request(std::size_t req_id, del_command &&cmd);

    MSGPACK_DEFINE_ARRAY(req_id, cmd);
};

/*
Participant.
*/

/// Used by participant.
struct db_request {
    /// NOTE: [req_id] is determined by the coordinator.
    std::size_t req_id;
    std::unique_ptr<command> cmd;

    db_request() = default;
    ~db_request() = default;

    db_request(db_request &&req);

    /// Move ctors.
    db_request(db_get_request &&req);
    db_request(db_set_request &&req);
    db_request(db_del_request &&req);
};


} // namespace simple_kv_store


#endif