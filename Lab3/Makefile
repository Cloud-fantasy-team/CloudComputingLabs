
LIB_TCP_SERVER_DIR = ${SRC_DIR}/tcp_server
LIB_TCP_SERVER = libtcp_server.a

.PHONY: ${LIB_TCP_SERVER}
${LIB_TCP_SERVER}:
	@ cd src/tcp_server/ && ${MAKE} LIB_TCP_SERVER=libtcp_server.a

.PHONY: clean
clean:
	rm -rf src/**/*.o src/**/*.a