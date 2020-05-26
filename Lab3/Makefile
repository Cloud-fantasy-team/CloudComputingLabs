CXX ?= g++
CXXFLAGS += -pthread -std=c++11
INCLUDE_PATH = -Ithird_party/rpclib/include -I/third_party/leveldb/include

OS := ""
UNAME = $(shell uname -s)

ifeq (${UNAME},Linux)
	OS = Linux
else
	ifeq (${UNAME},Darwin)
		OS = Darwin
	else
		$(error Non-supported OS)
	endif
endif

HEADERS = src/common.hpp src/command.hpp src/command_parser.hpp
SOURCES = src/command.cpp src/command_parser.cpp
OBJECTS = src/command.o src/command_parser.o

all: src/tcp_server/libtcp_server.a third_party/rpclib/librpc.a third_party/leveldb/libleveldb.a ${OBJECTS}
	@ echo build OK

%.o: %.cpp ${HEADERS}
	${CXX} -c ${CXXFLAGS} ${INCLUDE_PATH} -o $@ $<

src/tcp_server/libtcp_server.a:
	@ printf "building libtcp_server.a\n"
	@ cd src/tcp_server/ && ${MAKE}
	@ echo 

third_party/rpclib/librpc.a:
	@ printf "building librpc.a\n"
	@ cd third_party/rpclib/ && ${MAKE}
	@ echo

third_party/leveldb/libleveldb.a:
	@ printf "building libleveldb.a\n"
	@ cd third_party/leveldb/ && ${MAKE}
	@ echo

.PHONY: clean
clean:
	@ cd src/tcp_server && ${MAKE} clean
	@ cd third_party/rpclib && ${MAKE} clean
	@ cd third_party/leveldb && ${MAKE} clean
	@ rm ${OBJECTS}