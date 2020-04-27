#include <cstdlib>
#include <vector>
#include <iostream>
#include "thread_pool.h"

thread_pool pool(3);

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
        if (expected[i] != res[i].get())
        {
            std::cerr << "thread_pool_test.cc: simple() failed" << std::endl;
            exit(-1);
        }
    }
}

int main()
{
    simple();
}