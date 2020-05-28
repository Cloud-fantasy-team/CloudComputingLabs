#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <iostream>
#include "command_parser.hpp"
#include "message.hpp"
#include "rpc/client.h"
using namespace cdb;

// Single threaded. No need to synchronize.
std::vector<std::future<clmdep_msgpack::object_handle>> futures;

void make_set_rpc(db_set_request req, rpc::client &client)
{
    try {
        auto future = client.async_call("prepare_set", req);
        auto ok = future.get().as<bool>();
        // auto ok = client.call("prepare_set", req).as<bool>();
        if (ok)
        {
            auto future = client.async_call("commit", req.req_id);
            futures.emplace_back(std::move(future));
        }
        else
        {
            std::cout << "make_set_rpc - prepare failed" << std::endl;
        }
    } catch(std::exception &e) {
        std::cout << "make_set_rpc caught: " << e.what() << std::endl;
    }
}

void make_get_rpc(db_get_request req, rpc::client &client)
{
    try {
        std::string result = client.call("get", req).as<std::string>();
        std::cout << "> " << result << std::endl;
    } catch(std::exception &e) { std::cout << "make_get_rpc caught: " << e.what() << std::endl; }
}

void make_del_rpc(db_del_request req, rpc::client &client)
{
    try {
        auto future = client.async_call("prepare_del", req);
        auto ok = future.get().as<bool>();
        // auto ok = client.call("prepare_del", req).as<bool>();
        if (ok)
        {
            auto future = client.async_call("commit", req.req_id);
            futures.emplace_back(std::move(future));
        }
        else
        {
            std::cout << "make_del_rpc - prepare failed" << std::endl;
        }
    } catch(std::exception &e) { std::cout << "make_del_rpc caught: " << e.what() << std::endl; }
}

/// A script for testing RPC functionalities.
int main()
{
    /// WARN:
    /// We can have at most two concurrent/parallel update requests to be safe,
    /// since we're using 2 callback workers by default.
    /// Now we're sending 3 update requests concurrently. Note that the participant
    /// will probably crash.
    db_set_request set_req3{2, {"CS06142", "Cloud Computing 20"}};
    db_set_request set_req{0, {"CS06142", "Cloud Computing"}};
    db_del_request del_req{1, {std::vector<std::string>{"CS06142", "non-existing"}}};
    
    db_get_request get_req{4, {"CS06142"}};
    db_get_request get_req2{5, {"non_existing"}};
    rpc::client client{"127.0.0.1", 8001};
    client.set_timeout(200);

    make_set_rpc(set_req, client);
    // make_set_rpc(set_req3, client);
    make_del_rpc(del_req, client);

    for (auto &future : futures)
    {
        try {
            auto ret = future.get().as<std::string>();
            std::cout << "> " << ret << std::endl;
        } catch(std::exception &e) {
            std::cout << "caught: " << e.what() << std::endl;
        }
    }

    // make_get_rpc(get_req, client);
    // make_get_rpc(get_req2, client);

    for (;;) {}
}