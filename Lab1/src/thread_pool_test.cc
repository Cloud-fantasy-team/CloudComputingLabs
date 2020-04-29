#include <cstdlib>
#include <vector>
#include <iostream>
#include "thread_pool.h"

thread_pool pool(std::thread::hardware_concurrency());

void simple()
{
    auto addition = [](int a, int b) { return a + b; };

    std::vector<std::pair<int, int>> params{
        {23, 23},
        {10, 23},
        {33, 3}
    };
    std::vector<int> expected{
        23 + 23,
        10 + 23,
        33 + 3
    };
    std::vector<std::future<int>> res;

    for (auto &p : params)
    {
        res.push_back(pool.add_task(addition, p.first, p.second));
    }

    for (int i = 0; i < res.size(); i++)
    {
        assert(expected[i] == res[i].get() && "thread_pool_test.cc: simple() failed");
    }
}

int main()
{
    simple();
}