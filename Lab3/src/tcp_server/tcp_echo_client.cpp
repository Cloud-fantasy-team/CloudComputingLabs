#include <functional>
#include <iostream>
#include <condition_variable>
#include <vector>
#include <string>
#include "tcp_client.hpp"

using tcp_server_lib::tcp_client;

std::condition_variable cond;
std::mutex mutex;

static void on_read_completion(tcp_client &c, tcp_client::read_result &result)
{
    std::cout << "recv'ed from echo server" << std::endl;
    std::cout << std::string{result.data.begin(), result.data.end()} << std::endl << std::endl;

    c.disconnect(true);
}

static void on_write_completion(tcp_client &client, tcp_client::write_result &result)
{
    std::cout << "sent " << result.size << " bytes to server" << std::endl;
    client.async_read({
        result.size,
        std::bind(&on_read_completion, std::ref(client), std::placeholders::_1)
    });
}

int main()
{
    tcp_client client;

    client.connect("localhost", 3001);
    std::vector<std::string> lines{ 
        {"some random string 1."}, 
        {"complete garbled"},
        {"season system developer"}
    };

    client.on_disconnection() = [&]() { 
        std::cout << "client disconnected" << std::endl;
        client.get_reactor()->stop();
        cond.notify_all();
        exit(0);
    };

    for (auto &line : lines)
        client.async_write({
            std::vector<char>{line.begin(), line.end()},    /* data */
            std::bind(&on_write_completion, std::ref(client), std::placeholders::_1)
        });

    // Wait for ^C
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock);

    exit(0);
}