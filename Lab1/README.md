Lab 1: "Super fast" sudoku solving
----------------------------------

This is the first lab of the course *HNU Cloud Computing: Spring 2020*. See the repo [CloudComputingLabs/Lab1](https://github.com/1989chenguo/CloudComputingLabs/tree/master/Lab1) for a complete description. 

Structure
---------

* `scripts/`: debugging scripts.
* `src/`: all source code for this lab.
    * `common.h`: common configurations.
    * `sudoku_basic.h`: a basic DFS backtracking method for sudoku solving.
    * `thread_pool.h`: a simple symmetric thread pool.
    * `main.cc`: the main program that leverages thread_pool to solve sudoku's concurrently from the input files.
    * `*_test.cc`: specific unit tests for each component.
* `sudoku_testcases/`: sudoku test cases.

Current status
--------------

The main program should be able to read input file name from stdin and solve the puzzles in each of the given file. 