
LIB_TCP_SERVER_DIR = src/tcp_server
LIB_TCP_SERVER = libtcp_server.a

.PHONY: ${LIB_TCP_SERVER}
${LIB_TCP_SERVER}:
	@ cd src/tcp_server/ && ${MAKE}

.PHONY: clean
clean:
	rm -rf src/**/*.o src/**/*.a