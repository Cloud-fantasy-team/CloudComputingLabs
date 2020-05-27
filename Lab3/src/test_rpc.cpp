#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <iostream>
#include "command_parser.hpp"
#include "message.hpp"
#include "rpc/client.h"
using namespace simple_kv_store;

std::mutex mutex;
std::vector<std::future<clmdep_msgpack::object_handle>> futures;

void make_set_rpc(db_set_request req, rpc::client &client)
{
    try {
        auto ok = client.call("prepare_set", req).as<bool>();
        if (ok)
        {
            auto future = client.async_call("commit", req.req_id);
            std::unique_lock<std::mutex> lock(mutex);
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
        auto ok = client.call("prepare_del", req).as<bool>();
        if (ok)
        {
            auto future = client.async_call("commit", req.req_id);
            std::unique_lock<std::mutex> lock(mutex);
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
    db_set_request set_req3{2, {"CS06142", "Cloud Computing 20"}};
    db_set_request set_req{0, {"CS06142", "Cloud Computing"}};
    db_del_request del_req{1, {std::vector<std::string>{"CS06142", "non-existing"}}};
    
    db_get_request get_req{4, {"CS06142"}};
    db_get_request get_req2{5, {"non_existing"}};
    rpc::client client{"127.0.0.1", 8080};

    make_set_rpc(set_req3, client);
    make_set_rpc(set_req, client);
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

    make_get_rpc(get_req, client);
    make_get_rpc(get_req2, client);
}