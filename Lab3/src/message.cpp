#include "message.hpp"

namespace simple_kv_store {

/*
coordinator.
*/
db_get_request::db_get_request(std::size_t req_id, get_command &&cmd)
    : req_id(req_id), cmd(std::move(cmd)) {}

db_set_request::db_set_request(std::size_t req_id, set_command &&cmd)
    : req_id(req_id), cmd(std::move(cmd)) {}

db_del_request::db_del_request(std::size_t req_id, del_command &&cmd)
    : req_id(req_id), cmd(std::move(cmd)) {}


/*
participant.
*/
db_request::db_request(db_request &&req)
    : req_id(req.req_id)
{
    cmd = std::move(req.cmd);
}

db_request::db_request(db_get_request &&req)
    : req_id(req.req_id)
{
    cmd.reset(new get_command(std::move(req.cmd)));
}

db_request::db_request(db_set_request &&req)
    : req_id(req.req_id)
{
    cmd.reset(new set_command(std::move(req.cmd)));
}

db_request::db_request(db_del_request &&req)
    : req_id(req.req_id)
{
    cmd.reset(new del_command(std::move(req.cmd)));
}

}    // namespace simple_kv_store