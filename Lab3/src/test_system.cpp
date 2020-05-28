#include <signal.h>
#include <condition_variable>
#include <mutex>
#include <string>
#include <iostream>
#include "tcp_server/tcp_client.hpp"
using tcp_server_lib::tcp_client;

std::mutex mutex;
std::condition_variable cond;

void get_result(tcp_client &client, tcp_client::write_result &result)
{
    if (result.success)
    {
        std::cout << "sent " << result.size << " bytes\n";
        try {
            client.async_read({
                1024,
                [&](tcp_client::read_result &result) {
                    if (!result.success)
                    {
                        std::cout << "Failed reading from server" << std::endl;
                        return;
                    }

                    std::string ret{result.data.begin(), result.data.end()};
                    std::cout << "> " << ret << std::endl;
                }
            });
        } catch (std::runtime_error &e)
        {
            std::cout << "get_result caught: " << e.what() << std::endl;
        }
    }
    else
        std::cout << "write failed" << std::endl;
}

int main()
{
    std::string del_cmd{"*3\r\n$3\r\nDEL\r\n$7\r\nCS06142\r\n$5\r\nCS162\r\n"};
    std::string get_cmd{"*2\r\n$3\r\nGET\r\n$7\r\nCS06142\r\n"};
    std::string set_cmd{"*4\r\n$3\r\nSET\r\n$7\r\nCS06142\r\n$5\r\nCloud\r\n$9\r\nComputing\r\n"};

    std::vector<std::string> cmds{ set_cmd, get_cmd, del_cmd, get_cmd };

    tcp_client client;
    try 
    {
        client.connect("localhost", 8080);

        for (std::string const& cmd : cmds)
        {
            client.async_write({
                std::vector<char>{cmd.begin(), cmd.end()},
                std::bind(&get_result, std::ref(client), std::placeholders::_1)
            });
        }
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Caught error: " << e.what() << std::endl;
    }

    for(;;) {}
}