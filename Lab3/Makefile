CXX ?= g++
CXXFLAGS += -pthread -std=c++11

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

all: librpc.a libtcp_server.a ${OBJECTS}
	@ echo build OK

%.o: %.cpp ${HEADERS}
	${CXX} -c ${CXXFLAGS} -o $@ $<

.PHONY: libtcp_server.a
libtcp_server.a:
	@ printf "building libtcp_server.a\n"
	@ cd src/tcp_server/ && ${MAKE}
	@ echo 

.PHONY: librpc.a
librpc.a:
	@ printf "building librpc.a\n"
	@ cd third_party/rpclib/ && ${MAKE}
	@ echo

.PHONY: clean
clean:
	@ cd src/tcp_server && ${MAKE} clean
	@ cd third_party/rpclib && ${MAKE} clean
	@ rm ${OBJECTS}