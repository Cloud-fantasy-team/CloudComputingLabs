#include <iostream>
#include <getopt.h>
#include "server.h"
#include "tcp_socket.h"
#include "reporter.h"
using namespace simple_http_server;

static struct option long_options[] = {
    { "ip", required_argument, NULL, 'i' },
    { "port", required_argument, NULL, 'p' },
    { "proxy", required_argument, NULL, 'x' },
    { "number-thread", required_argument, NULL, 'n' },
    { 0, 0, 0, 0 }
};

static void print_usage(char *const prog)
{
    std::cerr << "Usage: " << prog << " [OPTION]" << std::endl;
    std::cerr << "[OPTION] can be the following:" << std::endl;
    std::cerr << "\t" << "--ip " << "ip_address" << std::endl;
    std::cerr << "\t" << "--port " << "port_number" << std::endl;
    std::cerr << "\t" << "--proxy " << "proxy_address" << std::endl;
    std::cerr << "\t" << "--number-thread " << "n" << std::endl;
}

int main(int argc, char *const argv[])
{
    std::string ip = "127.0.0.1";
    uint16_t port = 8888;
    size_t thread_num = 8;

    int oc;
    while ((oc = getopt_long(argc, argv, ":", long_options, NULL)) != -1) {
        switch (oc) {
        case 'i':
            ip = std::string(optarg); 
            break;
        case 'p':
            port = std::stoi(std::string(optarg)); 
            break;
        case 'x':
            std::cerr << "proxy mode is currently not supported" << std::endl;
            abort();
        case 'n':
            thread_num = std::stoul(std::string(optarg)); 
            break;
        default:
            print_usage(argv[0]);
            break;
        }
    }

    initialize_reporter("err_server.log", "warn_server.log", "info_server.log");
    Server server(ip, port, "", thread_num);
    server.start();
}
