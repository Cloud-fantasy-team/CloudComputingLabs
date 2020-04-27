#include <string>
#include <fstream>
#include <iostream>
#include <exception>
#include <thread>
#include <functional>

#include "sudoku_basic.h"
#include "thread_pool.h"

/// Exception thrown by failing to open a file.
class bad_filename : public std::exception
{
public:
    bad_filename(const std::string &f) : name(f) {}

    virtual const char *what() const throw()
    {
        return (std::string{ "bad filename: " } + name).c_str();
    }

private:
    std::string name;
};

/// Synchronously read lines from [filename] and pass each line to [process_line]
/// eagerly.
template <typename F>
void process_file_by_line(std::string filename, F &&process_line)
{
    auto fs = std::make_unique<std::ifstream>(filename, std::ios::in);

    // FIXME: shutdown gracefully instead of terminating the program
    // abruptly.
    if (!fs || !fs->is_open())
    {
        throw bad_filename(filename);
    }

    /// Exhaust the file.
    std::string line;
    while (std::getline(*fs, line))
    {
        process_line(line);
    }
}

/// Deserialization of the board.
static void deserialize_board(const std::string &line, int board[DIM][DIM])
{
    assert(line.length() == 81);
    int *board_ptr = reinterpret_cast<int*>(board);

    for (char ch : line)
    {
        assert(ch >= '0' && ch <= '9' && "invalid sudoku board");
        *board_ptr++ = ch - '0';
    }
}

/// Serialization.
static std::string serialize_board(int board[DIM][DIM])
{
    std::string ret;

    ret.reserve(81);
    for (int i = 0; i < DIM; i++)
    {
        for (int j = 0; j < DIM; j++)
        {
            ret.push_back('0' + board[i][j]);
        }
    }

    return ret;
}

int main()
{
    // TODO: add support for specifying the number of threads
    // in the pool.
    thread_pool pool(std::thread::hardware_concurrency());

    // Result of solved sudoku's. This queue is necessary because we'll have to
    // ensure to output in input-file order.
    std::vector<std::future<std::string>> res;

    // Main thread performs reading input file name from stdin,
    // and creates tasks for the pool.
    std::string filename;
    for (;;)
    {
        // Flush all solved sudoku's, meaning that we'll wait.
        for (auto iter = res.begin(); iter != res.end(); iter++)
        {
            std::string board = iter->get();
            std::cout << board << std::endl;
        }
        res.clear();

        // Wait for filename
        if (!std::getline(std::cin, filename))
            break;

        // Add the whole task to thread pool.
        process_file_by_line(filename, [&pool, &res](std::string line) {
            auto f = pool.add_task([](std::string line) -> std::string
            {
                int board[DIM][DIM];

                deserialize_board(line, board);
                if (solve_sudoku_basic(board))
                {
                    return serialize_board(board);
                }
                
                return std::string{""};
            }, line);

            // Collect the future.
            res.emplace_back(std::move(f));
        });
    }

    // Flush remaining board.
    for (auto iter = res.begin(); iter != res.end(); iter++)
    {
        std::string board = iter->get();
        std::cout << board << std::endl;
    }
    res.clear();
}