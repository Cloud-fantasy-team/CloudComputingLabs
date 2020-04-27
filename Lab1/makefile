CC=g++
CXXFLAGS=-std=c++14 -Wall -Werror

sudoku_basic_test: sudoku_basic_test.o sudoku_basic.o
	$(CC) $(CXXFLAGS) sudoku_basic_test.o sudoku_basic.o -o sudoku_basic_test
	./sudoku_basic_test

thread_pool_test: src/thread_pool_test.cc src/thread_pool.h
	@ printf "Compiling and testing thread pool...\n"
	@ $(CC) $(CXXFLAGS) src/thread_pool_test.cc -lpthread -o thread_pool_test
	./thread_pool_test

%.o : src/%.cc
	$(CC) -c $<

clean:
	@ rm -rf ./*_test
	@ rm -rf ./*.o