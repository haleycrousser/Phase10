////////// SERVER HEADERS //////////
//Created cross-platform code, no reason just thought why not!
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") 
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#ifndef _WIN32
    typedef int SOCKET;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define closesocket(s) close(s)
    #include <errno.h>
#endif

/* Notes For Myself (regarding server funcs)

socket() - 
bind() - 
listen() -

getaddrinfo() - 

*/

////////// HEADERS //////////
#include <cstring>
#include <iostream>
#include <string>

////////// FILES //////////
#include "gameLoop\card.h"

////////// FUNCTION PROTOTYPES //////////
SOCKET createServer();

int main() {

    // Create the server socket
    SOCKET serverfd = createServer();
    if (serverfd == INVALID_SOCKET) {
        std::cerr << "Failed to create server\n";
        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }


////////// GAME STUFF //////////
    std::cout << "Server listening on port 8080...\n";

    // Step 4: Wait for and accept an incoming client connection
    // =========================================================
    // accept() blocks (waits) here until a client connects
    // When a client connects, it creates a NEW socket for communicating with that specific client
    // The original serverfd continues to listen for more connections
    // The two nullptr arguments mean we don't want to know client's address or its length
    SOCKET clientfd = accept(serverfd, nullptr, nullptr);
    if (clientfd == INVALID_SOCKET) {
        // accept() failed
        #ifdef _WIN32
            std::cerr << "accept failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "accept failed with error: " << strerror(errno) << "\n";
        #endif
        closesocket(serverfd);  // Close the listening socket
        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }
    std::cout << "Client connected!\n";

    // Step 5: Send a question to the client
    // =====================================
    const char *question = "What is your name? ";
    // send() transmits data to the client over the clientfd socket
    // The third argument is the number of bytes to send (strlen = string length)
    // The last argument (0) means no special flags
    // Returns number of bytes actually sent, or SOCKET_ERROR on failure
    if (send(clientfd, question, (int)strlen(question), 0) == SOCKET_ERROR) {
        #ifdef _WIN32
            std::cerr << "send failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "send failed with error: " << strerror(errno) << "\n";
        #endif
    }

    // Step 6: Receive the client's response
    // =====================================
    char buf[256] = {};  // Create a buffer to hold incoming data; {} initializes all bytes to 0
    // recv() waits to receive data from the client on the clientfd socket
    // sizeof(buf) - 1 leaves room for a null terminator at the end
    // The last argument (0) means no special flags
    // Returns number of bytes received, 0 if connection closed, SOCKET_ERROR on error
    int recvResult = recv(clientfd, buf, sizeof(buf) - 1, 0);
    if (recvResult > 0) {
        // Successfully received data
        buf[recvResult] = '\0';  // Add null terminator so buf is a valid C string
        std::cout << "Client's name: " << buf << "\n";
    } else if (recvResult == 0) {
        // Client closed the connection gracefully (0 bytes received = connection closed)
        std::cout << "Connection closed\n";
    } else {
        // recv() returned SOCKET_ERROR (negative value)
        #ifdef _WIN32
            std::cerr << "recv failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "recv failed with error: " << strerror(errno) << "\n";
        #endif
    }


////////// CLEAN UP //////////
    // Step 7: Clean up and close all sockets
    // ======================================
    closesocket(clientfd);   // Close the socket connected to the client
    closesocket(serverfd);   // Close the listening socket
    #ifdef _WIN32
        WSACleanup();            // Shut down Winsock and release resources (Windows only)
    #endif
    return 0;  // Return 0 to OS indicating successful execution
}

SOCKET createServer() {
    // Initialize Windows Sockets subsystem (Windows only)
    #ifdef _WIN32
        WSADATA wsaData;
        int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaErr != 0) {
            std::cerr << "WSAStartup failed: " << wsaErr << "\n";
            return INVALID_SOCKET;
        }
    #endif

    // Step 1: Set up address information for the server
    // ==================================================
    struct addrinfo hints{}, *res;  // hints tells getaddrinfo what we want; res holds the result
    // Initialize all fields to zero with {} to avoid garbage data
    hints.ai_family   = AF_INET;       // Use IPv4 address family
    hints.ai_socktype = SOCK_STREAM;   // Use TCP (stream) sockets, not UDP (datagram)
    hints.ai_flags    = AI_PASSIVE;    // Bind to any available network interface (0.0.0.0)

    // getaddrinfo() looks up address info for localhost on port 8080
    // nullptr means bind to any local address
    // "8080" is the port we want the server to listen on
    // If successful, 'res' will point to address structure we can use
    if (getaddrinfo(nullptr, "8080", &hints, &res) != 0) {
        std::cerr << "getaddrinfo failed\n";
        return INVALID_SOCKET;
    }

    // Step 2: Create a server socket
    // ==============================
    // socket() creates a socket using the address family (IPv4), socket type (TCP), and protocol
    // Returns a SOCKET handle (identifier) we use for all future operations on this socket
    SOCKET serverfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverfd == INVALID_SOCKET) {
        // socket() failed; INVALID_SOCKET is what Windows returns on error
        #ifdef _WIN32
            std::cerr << "socket failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "socket failed with error: " << strerror(errno) << "\n";
        #endif
        freeaddrinfo(res);  // Free memory allocated by getaddrinfo()
        return INVALID_SOCKET;
    }

    // Configure socket behavior
    int opt = 1;
    // SO_REUSEADDR allows the server to restart immediately without waiting for TIME_WAIT
    // Without this, you might get "Address already in use" errors if restarting quickly
    #ifdef _WIN32
        if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
    #else
        if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR) {
    #endif
        #ifdef _WIN32
            std::cerr << "setsockopt failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "setsockopt failed with error: " << strerror(errno) << "\n";
        #endif
        closesocket(serverfd);  // Close the socket we created
        freeaddrinfo(res);      // Free address info
        return INVALID_SOCKET;
    }

    // Step 3: Bind socket to address and port, then start listening
    // ==============================================================
    // bind() associates the socket with the address and port from getaddrinfo() result
    // This tells the OS "when packets arrive for port 8080, give them to this socket"
    if (bind(serverfd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        #ifdef _WIN32
            std::cerr << "bind failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "bind failed with error: " << strerror(errno) << "\n";
        #endif
        closesocket(serverfd);
        freeaddrinfo(res);
        return INVALID_SOCKET;
    }

    // listen() marks the socket as a listening socket, ready to accept incoming connections
    // The second argument (10) is the backlog: max number of pending connections to queue
    // Clients trying to connect when backlog is full will get a "connection refused" error
    if (listen(serverfd, 10) == SOCKET_ERROR) {
        #ifdef _WIN32
            std::cerr << "listen failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "listen failed with error: " << strerror(errno) << "\n";
        #endif
        closesocket(serverfd);
        freeaddrinfo(res);
        return INVALID_SOCKET;
    }
    // We're done with the address info, so free the memory getaddrinfo() allocated
    freeaddrinfo(res);

    return serverfd;
}