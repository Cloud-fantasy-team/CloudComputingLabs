CC=g++
CXXFLAGS=-std=c++14 -Wall -Werror

all: test sudoku_solve

test: sudoku_basic_test thread_pool_test
	./thread_pool_test
	./sudoku_basic_test

sudoku_solve: main.o
	@ $(CC) $(CXXFLAGS) -o $@ $^

sudoku_basic_test: sudoku_basic_test.o
	@ $(CC) $(CXXFLAGS) -o $@ $^

thread_pool_test: thread_pool_test.o
	@ $(CC) $(CXXFLAGS) $^ -lpthread -o $@

%.o: src/%.cc
	@ $(CC) $(CXXFLAGS) -c $<

clean:
	@ rm -rf ./*.o
	@ rm -rf ./*test
	@ rm -rf ./sudoku_solve