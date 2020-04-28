CC=g++
CXXFLAGS=-std=c++11 -Wall -Werror
LDFLAGS=-lpthread

SRC=$(shell find ./src -type f -name *.cc)
HEADERS=$(shell find ./src -type f -name *.h)
OBJS := $(patsubst ./src/%.cc, ./src/%.o, $(SRC))

TARGET=httpserver

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJS): ./src/%.o: ./src/%.cc ./src/%.h
	$(CC) -g -c $(CXXFLAGS) -o $@ $<

# Empty rule.
src/main.h:

clean:
	rm -rf $(OBJS) $(TARGET)