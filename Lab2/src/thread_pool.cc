/// thread_pool.cc
/// Copyright 2020 Cloud-fantasy team

#include "server.h"
#include "thread_pool.h"

namespace simple_http_server
{

thread_pool::thread_pool(Server &server, size_t size)
:   server(server), done(false)
{
    // Initialize each worker.
    for (size_t i = 0; i < size; i++)
    {
        workers.emplace_back(
            // Thread object.
            [this]
            {
                for (;;)
                {
                    std::unique_ptr<TCPSocket> client_sock;
                    {
                        // Acquire lock.
                        std::unique_lock<std::mutex> lock(this->m);
                        this->condition.wait(
                            lock, 
                            [this]{ return !this->client_socks.empty() || this->done; }
                        );

                        // When done is set by the destructor, exit this worker thread.
                        if (this->done)
                            return;

                        // Pull task from the queue.
                        client_sock = std::move(this->client_socks.front());
                        this->client_socks.pop();
                    }

                    // Call back to server.
                    this->server.serve_client(std::move(client_sock));
                }
            }
        );
    }
}

thread_pool::~thread_pool()
{
    // Set done.
    {
        std::unique_lock<std::mutex> lock(m);
        done = true;
    }

    // Reap child threads.
    condition.notify_all();
    for (auto &t : workers)
        t.join();
}

void thread_pool::add_client(std::unique_ptr<TCPSocket> sock)
{
    std::unique_lock<std::mutex> lock(m);
    client_socks.push(std::move(sock));

    condition.notify_one();
}

} // namespace simple_http_server
