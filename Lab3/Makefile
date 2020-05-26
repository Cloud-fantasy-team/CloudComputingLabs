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

HEADERS = src/command.hpp
SOURCES = src/command.cpp
OBJECTS = src/command.o

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