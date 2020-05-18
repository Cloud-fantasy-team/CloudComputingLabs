CC = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror

# I've got complaints about ${MAKE} required sudo privilege. Thought it
# probably caused by $(shell find ...) or whatever wildcard command. Though
# I couldn't replay it from any of my servers. So I listed all the source files here.
HEADERS = exceptions.hpp tcp_socket.hpp thread_pool.hpp pipe.hpp reactor.hpp tcp_client.hpp
SRCS = exceptions.cpp tcp_socket.cpp thread_pool.cpp pipe.cpp reactor.hpp tcp_client.cpp
OBJS = exceptions.o tcp_socket.o thread_pool.o pipe.o reactor.o tcp_client.o

LIB_TCP_SERVER ?= libtcp_server.a

.PHONY: ${LIB_TCP_SERVER}
${LIB_TCP_SERVER}: ${OBJS}
	ar -rc ${LIB_TCP_SERVER} $^

tcp_echo_client: ${LIB_TCP_SERVER}
	${CC} ${CXXFLAGS} tcp_echo_client.cpp -L. -ltcp_server -o tcp_echo_client

%.o: %.cpp ${HEADERS}
	${CC} -c ${CXXFLAGS} -o $@ $<

.PHONY: clean
clean:
	@ rm -rf *.o ${LIB_TCP_SERVER} tcp_echo_client
