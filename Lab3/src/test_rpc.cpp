#include <string>
#include <vector>
#include <iostream>
#include "command_parser.hpp"
#include "message.hpp"
#include "rpc/client.h"
using namespace simple_kv_store;

/// A script for testing RPC functionalities.
int main()
{
    set_command set_cmd{"new_key", "new_value"};

    db_set_request set_req{1, std::move(set_cmd)};
    rpc::client client{"127.0.0.1", 8080};

    try {
        bool ok = client.call("prepare_set", set_req).as<bool>();
        if (ok) {
            std::string result = client.call("commit", set_req.req_id).as<std::string>();
            std::cout << "> " << result << std::endl;
        }
        else
            std::cout << "prepare_set failed" << std::endl;
    } catch(std::exception &e) { std::cout << "Caught: " << e.what() << std::endl; }
}