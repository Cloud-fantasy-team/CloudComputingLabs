CC = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror

HEADERS = exceptions.hpp tcp_socket.hpp
SRCS = exceptions.cpp tcp_socket.cpp
OBJS = exceptions.o tcp_socket.o

LIB_NAME = libtcpserver

.PHONY: ${LIB_NAME}
${LIB_NAME}: ${OBJS}
	ar -rc libtcpserver.a $<

%.o: %.cpp ${HEADERS}
	${CC} -c ${CXXFLAGS} -o $@ $<

.PHONY: clean
clean:
	@ rm -rf *.o ${LIB_NAME}
