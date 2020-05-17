CC = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror

HEADERS = exceptions.hpp tcp_socket.hpp thread_pool.hpp
SRCS = exceptions.cpp tcp_socket.cpp thread_pool.cpp
OBJS = exceptions.o tcp_socket.o thread_pool.o

LIB_TCP_SERVER ?= libtcp_server.a

.PHONY: ${LIB_TCP_SERVER}
${LIB_TCP_SERVER}: ${OBJS}
	ar -rc ${LIB_TCP_SERVER} $^

%.o: %.cpp ${HEADERS}
	${CC} -c ${CXXFLAGS} -o $@ $<

.PHONY: clean
clean:
	@ rm -rf *.o ${LIB_TCP_SERVER}
