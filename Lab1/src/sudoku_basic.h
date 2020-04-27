#include <cassert>
#include <vector>
#include "common.h"

bool is_safe(int board[DIM][DIM], int r, int c, int v)
{
    // Ensure no dup in row.
    for (int col = 0; col < DIM; col++)
    {
        if (board[r][col] == v)
            return false;
    }

    // Ensure no dup in col.
    for (int row = 0; row < DIM; row++)
    {
        if (board[row][c] == v)
            return false;
    }

    // Ensure no dup in box.
    int box_start_r = r - r % 3;
    int box_start_c = c - c % 3;

    for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			if (board[row + box_start_r][col + box_start_c] == v) 
			{
				return false;
			}

    return true;
}

std::pair<int, int> get_unassigned_location(int board[DIM][DIM])
{
	for (int row = 0; row < DIM; row++)
		for (int col = 0; col < DIM; col++)
			if (board[row][col] == 0)
			{
				return std::make_pair(row, col);
			}

	return std::make_pair(9, 9);
}

/// Sodoku solving using basic DFS backtracking.
bool solve_sudoku_basic(int board[DIM][DIM])
{
    // Check if we've done.
    if (std::make_pair(9, 9) == get_unassigned_location(board))
    {
        return true;
    }

    auto p = get_unassigned_location(board);
    int row = p.first;
    int col = p.second;

    for (int v = 1; v <= 9; v++)
    {
        if (is_safe(board, row, col, v))
        {
            board[row][col] = v;

            if (solve_sudoku_basic(board))
                return true;

            // Backtrack.
            board[row][col] = 0;
        }
    }

    return false;
}