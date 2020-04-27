#include <iostream>
#include "reporter.h"
using namespace simple_http_server;

int main(int argc, char **argv)
{
    initialize_reporter("err.txt", "warn.txt", "info.txt");
}
