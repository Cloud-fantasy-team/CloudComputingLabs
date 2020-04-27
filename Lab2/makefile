CC=g++
CXXFLAGS=-std=c++11 -Wall -Werror
LDFLAGS=-lpthread

SRC=$(shell find ./src -type f -name *.cc)
HEADERS=$(shell find ./src -type f -name *.h)
OBJS=$(SRC:.cc=.o)

TARGET=httpserver

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(TARGET)