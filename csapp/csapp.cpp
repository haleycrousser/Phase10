// csapp.cpp
#include "csapp.h"
 
// ================================================================
//  SERVER FUNCTIONS
// ================================================================
 
SOCKET createServer(const char *port) {
    #ifdef _WIN32
        WSADATA wsaData;
        int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaErr != 0) {
            std::cerr << "WSAStartup failed: " << wsaErr << "\n";
            return INVALID_SOCKET;
        }
    #endif
 
    struct addrinfo hints{}, *res;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
 
    if (getaddrinfo(nullptr, port, &hints, &res) != 0) {
        NET_ERR("getaddrinfo failed");
        return INVALID_SOCKET;
    }
 
    SOCKET serverfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverfd == INVALID_SOCKET) {
        NET_ERR("socket failed");
        freeaddrinfo(res);
        return INVALID_SOCKET;
    }
 
    int opt = 1;
    #ifdef _WIN32
        setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    #else
        setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif
 
    if (bind(serverfd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        NET_ERR("bind failed");
        closesocket(serverfd);
        freeaddrinfo(res);
        return INVALID_SOCKET;
    }
 
    if (listen(serverfd, 10) == SOCKET_ERROR) {
        NET_ERR("listen failed");
        closesocket(serverfd);
        freeaddrinfo(res);
        return INVALID_SOCKET;
    }
 
    freeaddrinfo(res);
    return serverfd;
}
 
SOCKET listenForClient(SOCKET serverfd) {
    std::cout << "Server listening...\n";
 
    SOCKET clientfd = accept(serverfd, nullptr, nullptr);
    if (clientfd == INVALID_SOCKET) {
        NET_ERR("accept failed");
        closesocket(serverfd);
        #ifdef _WIN32
            WSACleanup();
        #endif
        exit(1);
    }
 
    std::cout << "Client connected!\n";
    return clientfd;
}
 
void closeServer(SOCKET clientfd, SOCKET serverfd) {
    closesocket(clientfd);
    closesocket(serverfd);
    #ifdef _WIN32
        WSACleanup();
    #endif
}
 
// ================================================================
//  CLIENT FUNCTIONS
// ================================================================
 
SOCKET connectToServer(const char *host, const char *port) {
    #ifdef _WIN32
        WSADATA wsaData;
        int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaErr != 0) {
            std::cerr << "WSAStartup failed: " << wsaErr << "\n";
            return INVALID_SOCKET;
        }
    #endif
 
    struct addrinfo hints{}, *res;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
 
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        NET_ERR("getaddrinfo failed");
        return INVALID_SOCKET;
    }
 
    SOCKET sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == INVALID_SOCKET) {
        NET_ERR("socket failed");
        freeaddrinfo(res);
        return INVALID_SOCKET;
    }
 
    if (connect(sockfd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        NET_ERR("connect failed");
        closesocket(sockfd);
        freeaddrinfo(res);
        return INVALID_SOCKET;
    }
 
    freeaddrinfo(res);
    return sockfd;
}
 
void closeConnection(SOCKET sockfd) {
    closesocket(sockfd);
    #ifdef _WIN32
        WSACleanup();
    #endif
}