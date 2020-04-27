#include <cassert>
#include <string>
#include <vector>
#include <iostream>

#include "common.h"

bool solve_sudoku_basic(std::vector<int> &board, int r, int c);

std::vector<int> read_input(const std::string &board_str)
{
    std::vector<int> board;
    board.reserve(kCol * kRow);
    
    for (auto d : board_str)
    {
        board.push_back(d - '0');
    }

    return board;
}

void simple()
{
    auto board = read_input("000000010400000000020000000000050407008000300001090000300400200050100000000806000");

    assert(solve_sudoku_basic(board, 0, 0));
}

int main()
{
    simple();
}