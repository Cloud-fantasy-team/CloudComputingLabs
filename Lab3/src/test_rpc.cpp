#include <string>
#include <vector>
#include <iostream>
#include "command_parser.hpp"
#include "message.hpp"
#include "rpc/client.h"
using namespace simple_kv_store;

void make_set_rpc(db_set_request req, rpc::client &client)
{
    try {
        bool ok = client.call("prepare_set", req).as<bool>();
        if (ok) {
            std::string result = client.call("commit", req.req_id).as<std::string>();
            std::cout << "> " << result << std::endl;
        }
        else {
            client.call("abort", req.req_id);
            std::cout << "prepare_set failed" << std::endl;
        }
    } catch(std::exception &e) { std::cout << "Caught: " << e.what() << std::endl; }
}

void make_del_rpc(db_del_request req, rpc::client &client)
{
    try {
        bool ok = client.call("prepare_del", req).as<bool>();
        if (ok) {
            std::string result = client.call("commit", req.req_id).as<std::string>();
            std::cout << "> " << result << std::endl;
        }
        else {
            client.call("abort", req.req_id);
            std::cout << "prepare_set failed" << std::endl;
        }
    } catch(std::exception &e) { std::cout << "Caught: " << e.what() << std::endl; }
}

/// A script for testing RPC functionalities.
int main()
{
    set_command set_cmd{"new_key", "new_value"};
    del_command del_cmd{std::vector<std::string>{"new_key", "non-existing"}};

    db_set_request set_req{1, std::move(set_cmd)};
    db_del_request del_req{2, std::move(del_cmd)};
    rpc::client client{"127.0.0.1", 8080};

    make_set_rpc(set_req, client);
    make_del_rpc(del_req, client);
}