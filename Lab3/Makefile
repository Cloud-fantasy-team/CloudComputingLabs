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

all: librpc.a libtcp_server.a
	@ echo build OK

.PHONY: libtcp_server.a
libtcp_server.a:
	@ printf "building libtcp_server.a\n"
	@ cd src/tcp_server/ && ${MAKE}
	@ echo 

.PHONY: librpc.a
librpc.a:
	@ printf "building librpc.a\n"
	@ cd src/rpclib/ && ${MAKE}
	@ echo

.PHONY: clean
clean:
	@ cd src/tcp_server && ${MAKE} clean
	@ cd src/rpclib && ${MAKE} clean