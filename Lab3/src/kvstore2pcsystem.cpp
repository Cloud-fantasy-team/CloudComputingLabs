#include <iostream>
#include "participant.hpp"
using simple_kv_store::participant;

int main()
{
    participant p{"127.0.0.1", 8080};
    p.start();
}