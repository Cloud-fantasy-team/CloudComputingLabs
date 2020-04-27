#include <cassert>
#include <vector>
#include "common.h"

#define INDEX(r, c) ((r) * kCol + (c))

/// Ensure board[r][c] can be filled with [v].
static bool ok(std::vector<int> &board, size_t r, size_t l, int v)
{
    size_t begin_row = r / kGrid * kGrid;
    size_t begin_col = l / kGrid * kGrid;

    // Ensure no dup in current row.
    for (size_t i = 0; i < l; i++)
    {
        if (board[INDEX(r, i)] == v)
            return false;
    }

    // Ensure no dup in current column.
    for (size_t i = 0; i < r; i++)
    {
        if (board[INDEX(i, l)] == v)
            return false;
    }

    // Ensure no dup in current grid.
    for (size_t i = begin_row; i <= r; i++)
    {
        for (size_t j = begin_col; j <= l; j++)
        {
            if (board[INDEX(i, j)] == v)    return false;
        }
    }

    return true;
}

/// Solve sudoku given from [board] in a DFS way.
bool solve_sudoku_basic(std::vector<int> &board, size_t r, size_t c)
{
    assert(board.size() == kCol * kRow);

    // Go to the next line if needed.
    if (c > kCol)
    {
        r = r + 1;
        c = 0;
    }

    // Check if we're done.
    if (INDEX(r, c) >= board.size())
        return true;

    // Try to find a value that fits in this cell.
    for (int v = 1; v <= kMax; v++)
    {
        if (ok(board, r, c, v))
        {
            board[INDEX(r, c)] = v;

            if (solve_sudoku_basic(board, r, c + 1))
                return true;

            // NOTE: reset is needed.
            board[INDEX(r, c)] = 0;
        }
    }

    return false;
}