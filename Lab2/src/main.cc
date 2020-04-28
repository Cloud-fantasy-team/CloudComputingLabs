#include <iostream>
#include "server.h"
#include "tcp_socket.h"
#include "reporter.h"
using namespace simple_http_server;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " mode" << std::endl;
        abort();
    }

    if (std::string(argv[1]) == "server")
    {
        initialize_reporter("err_server.log", "warn_server.log", "info_server.log");

        Server server("127.0.0.1", 8081);
        server.start();
    }

    else if (std::string(argv[1]) == "client")
    {
        initialize_reporter("err_client.log", "warn_client.log", "info_client.log");
        std::string line;

        while (true)
        {
            TCPSocket sock;
            sock.connect("127.0.0.1", 8081);
            std::getline(std::cin, line);
            sock.send_line(line);
            std::cout << sock.recv_line() << std::endl;
        }
    }
    else
    {
        abort();
    }
}
