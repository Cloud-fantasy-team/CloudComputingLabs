CC=g++
CXXFLAGS=-std=c++11 -Wall -Werror
LDFLAGS=-lpthread

SRC=$(shell find ./src -type f -name *.cc)
HEADERS=$(shell find ./src -type f -name *.h)
OBJS=$(SRC:.cc=.o)

TARGET=httpserver

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -rf $(OBJS) $(TARGET)