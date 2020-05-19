#include <condition_variable>
#include <mutex>
#include <iostream>
#include "tcp_server.hpp"
#include "tcp_client.hpp"

using tcp_server_lib::tcp_server;
using tcp_server_lib::tcp_client;

std::mutex mutex;
std::condition_variable cond;

static void echo(std::shared_ptr<tcp_client> client, tcp_client::read_result &result)
{
    if (result.success)
    {
        std::string client_data{result.data.begin(), result.data.end()};
        std::cout << "recv'ed " << client_data.size() << " bytes from client [" << client->host() << ":" << client->port() << "]" << std::endl;
        std::cout << "\t" << client_data << std::endl;

        client->async_write({
            result.data,
            [=](tcp_client::write_result &written) {
                std::cout << "\t\techo'ed " 
                          << written.size << " bytes "
                          << " to client [" << client->host() << ":" << client->port() << "]" << std::endl << std::endl;

                // Read again.
                client->async_read({
                    1024,
                    std::bind(&echo, client, std::placeholders::_1)
                });
            }
        });
    }

    // Else the client is diconnected.
}

int main()
{
    tcp_server server;

    server.start("127.0.0.1", 3001, [](std::shared_ptr<tcp_client> client) {
        client->on_disconnection() = [=]() {
            std::cout << "client [" << client->host() << ":" << client->port() << "] disconnected" << std::endl;
        };

        client->async_read({
            1024,   /* data size */
            std::bind(&echo, client, std::placeholders::_1)
        });
    });

    std::cout << "echo server listening at 127.0.0.1:3001" << std::endl;
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, []{ return false; });
}