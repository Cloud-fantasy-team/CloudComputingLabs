#include <functional>
#include <iostream>
#include <condition_variable>
#include <vector>
#include <string>
#include "tcp_client.hpp"

static void on_read_completion(tcp_server::tcp_client &, tcp_server::tcp_client::read_result &result)
{
    std::cout << "Recv from echo server" << std::endl;
    std::cout << std::string{result.data.begin(), result.data.end()} << std::endl << std::endl;
}

static void on_write_completion(tcp_server::tcp_client &client, tcp_server::tcp_client::write_result &result)
{
    client.async_read({
        result.size,
        std::bind(&on_read_completion, std::ref(client), std::placeholders::_1)
    });
}

int main()
{
    tcp_server::tcp_client client;

    client.connect("localhost", 3001);
    std::vector<std::string> lines{ 
        {"some random string 1."}, 
        {"complete garbled"},
        {"season system developer"}
    };
    for (auto &line : lines)
        client.async_write({
            std::vector<char>{line.begin(), line.end()},    /* data */
            std::bind(&on_write_completion, std::ref(client), std::placeholders::_1)
        });

    // Wait for ^C
    std::condition_variable cond;
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock);
}