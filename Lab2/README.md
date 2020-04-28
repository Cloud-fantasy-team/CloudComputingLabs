## Lab2 HTTP Server

- Requirements:
    - GNU Make.
    - C++ compiler with C++11 support.
    - POSIX-compliant operating system.

- Modules:
    - reporter: Logging singleton class.
    - message: HTTP request/response message classes.
    - thread_pool: Thread pool class designed for this server.
    - tcp_socket: wrapper for socket interfaces.
    - server: HTTP server class.

- Current status:
    [x] Basic version.
    [] Proxy mode.

Compiling and running the program is as described in the [instruction manual](https://github.com/1989chenguo/CloudComputingLabs/tree/master/Lab2#316-run-your-http-server).

```bash
make         # Compiling everything.
./httpserver # Default running on port 8888. Optional long args can be specified.
```
