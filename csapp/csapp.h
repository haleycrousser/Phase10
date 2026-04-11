#pragma once
 
// ── Platform headers ───────────────────────────────────────────
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int SOCKET;
    #define INVALID_SOCKET  (-1)
    #define SOCKET_ERROR    (-1)
    #define closesocket(s)  close(s)
#endif
 
#include <cstring>
#include <iostream>
#include <string>
 
// ── Error helper ───────────────────────────────────────────────
#ifdef _WIN32
    #define NET_ERR(msg) \
        std::cerr << msg << " (error " << WSAGetLastError() << ")\n"
#else
    #define NET_ERR(msg) \
        std::cerr << msg << ": " << strerror(errno) << "\n"
#endif
 
// ── Server functions ───────────────────────────────────────────
SOCKET createServer(const char *port = "8080");
SOCKET listenForClient(SOCKET serverfd);
void   closeServer(SOCKET clientfd, SOCKET serverfd);
 
// ── Client functions ───────────────────────────────────────────
SOCKET connectToServer(const char *host, const char *port);
void   closeConnection(SOCKET sockfd);