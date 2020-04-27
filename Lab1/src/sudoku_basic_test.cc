#include <cassert>
#include <string>
#include <vector>
#include <iostream>

#include "sudoku_basic.h"

std::vector<int> read_input(const std::string &str)
{
    std::vector<int> ret;
    ret.reserve(81);

    for (auto ch : str)
    {
        ret.push_back(ch - '0');
    }
    return ret;
}

void simple()
{
    int valid[DIM][DIM] = {
        {7, 2, 3, 0, 0, 0, 1, 5, 9},
        {6, 0, 0, 3, 0, 2, 0, 0, 8},
        {8, 0, 0, 0, 1, 0, 0, 0, 2},
        {0, 7, 0, 6, 5, 4, 0, 2, 0},
        {0, 0, 4, 2, 0, 7, 3, 0, 0},
        {0, 5, 0, 9, 3, 1, 0, 4, 0},
        {5, 0, 0, 0, 7, 0, 0, 0, 3},
        {4, 0, 0, 1, 0, 3, 0, 0, 6},
        {9, 3, 2, 0, 0, 0, 7, 1, 4}
    };

    int invalid[DIM][DIM] = {
        {7, 2, 3, 0, 0, 5, 1, 5, 9},
        {6, 0, 0, 3, 0, 2, 0, 0, 8},
        {8, 0, 0, 0, 1, 0, 0, 0, 2},
        {0, 7, 0, 6, 5, 4, 0, 2, 0},
        {0, 0, 4, 2, 0, 7, 3, 0, 0},
        {0, 5, 0, 9, 3, 1, 0, 4, 0},
        {5, 0, 0, 0, 7, 0, 0, 0, 3},
        {4, 0, 0, 1, 0, 3, 0, 0, 6},
        {9, 3, 2, 0, 0, 0, 7, 1, 4}
    };

    assert(solve_sudoku_basic(valid));
    assert(!solve_sudoku_basic(invalid));
}

int main()
{
    simple();
}