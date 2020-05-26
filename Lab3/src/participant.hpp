/// File participant.hpp
/// ====================
/// Copyright 2020 Cloud-fantasy team
/// This file contains the store server(participant) class.

#ifndef PARTICIPANT_HPP
#define PARTICIPANT_HPP

#include "leveldb/db.h"
#include "rpc/server.h"

namespace simple_kv_store {

/// Store server class. We're utilizing leveldb for real and fast storage.
class participant {
public:

private:
    /// The actual server.
    rpc::server svr;
};

} // namespace simple_kv_store


#endif