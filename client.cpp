// Include headers for cross-platform socket programming
#ifdef _WIN32
    #include <winsock2.h>      // Main Windows socket API header (socket(), connect(), etc.)
    #include <ws2tcpip.h>      // Extended socket functions (getaddrinfo(), etc.)
    #pragma comment(lib, "ws2_32.lib")  // Link Windows socket library at compile time
#else
    #include <sys/socket.h>    // POSIX socket functions
    #include <netdb.h>         // getaddrinfo() and related functions
    #include <unistd.h>        // close() function
    #include <errno.h>         // Error handling
#endif

#include <cstring>         // String functions like strlen()
#include <iostream>        // Output stream for printing (std::cout, std::cerr)
#include <string>          // Standard string class (std::string, std::getline())

// Cross-platform socket type definitions (for Unix/Linux, sockets are just integers)
#ifndef _WIN32
    typedef int SOCKET;         // On Unix/Linux, sockets are file descriptors (integers)
    #define INVALID_SOCKET (-1) // Invalid socket descriptor
    #define SOCKET_ERROR (-1)   // Socket operation error
    #define closesocket(s) close(s) // Use close() instead of closesocket()
#endif

int main() {
    // Step 0: Initialize Windows Sockets subsystem (Windows only)
    // ===========================================================
    #ifdef _WIN32
        WSADATA wsaData;  // Structure that receives details about Windows Sockets implementation
        // WSAStartup() initializes Winsock and must be called before any socket operations
        // MAKEWORD(2, 2) requests version 2.2 of the Winsock API
        // Returns 0 on success, non-zero error code on failure
        int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaErr != 0) {
            // If initialization failed, print error and exit
            std::cerr << "WSAStartup failed: " << wsaErr << "\n";
            return 1;  // Return error code to OS
        }
    #endif

    // Step 1: Look up server address information
    // ============================================
    struct addrinfo hints{}, *res;  // hints tells getaddrinfo what we want; res holds the result
    // Initialize all fields to zero with {} to avoid garbage data
    hints.ai_family   = AF_INET;       // Use IPv4 address family
    hints.ai_socktype = SOCK_STREAM;   // Use TCP (stream) sockets, not UDP (datagram)

    // getaddrinfo() looks up the server at 127.0.0.1 (localhost) on port 8080
    // If successful, 'res' will point to address structure we can use to connect
    if (getaddrinfo("127.0.0.1", "8080", &hints, &res) != 0) {
        std::cerr << "getaddrinfo failed\n";
        #ifdef _WIN32
            WSACleanup();  // Clean up Winsock before exiting (Windows only)
        #endif
        return 1;
    }

    // Step 2: Create a socket and connect to the server
    // ==================================================
    // socket() creates a socket using the address family (IPv4), socket type (TCP), and protocol
    // Returns a SOCKET handle (identifier) we use for all future operations on this socket
    SOCKET sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == INVALID_SOCKET) {
        // socket() failed; INVALID_SOCKET is what Windows returns on error
        #ifdef _WIN32
            std::cerr << "socket failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "socket failed with error: " << strerror(errno) << "\n";
        #endif
        freeaddrinfo(res);  // Free memory allocated by getaddrinfo()
        #ifdef _WIN32
            WSACleanup();       // Clean up Winsock (Windows only)
        #endif
        return 1;
    }

    // connect() establishes a connection to the server at the address from getaddrinfo()
    // This sends a connection request to the server and waits for acceptance
    // Returns 0 on success, SOCKET_ERROR on failure
    if (connect(sockfd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        #ifdef _WIN32
            std::cerr << "connect failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "connect failed with error: " << strerror(errno) << "\n";
        #endif
        closesocket(sockfd);  // Close the socket we created
        freeaddrinfo(res);    // Free address info
        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }
    // We're done with the address info, so free the memory getaddrinfo() allocated
    freeaddrinfo(res);

    // Step 3: Receive the question from the server
    // =============================================
    char buf[256] = {};  // Create a buffer to hold incoming data; {} initializes all bytes to 0
    // recv() waits to receive data from the server on the sockfd socket
    // sizeof(buf) - 1 leaves room for a null terminator at the end
    // The last argument (0) means no special flags
    // Returns number of bytes received, 0 if connection closed, SOCKET_ERROR on error
    int recvResult = recv(sockfd, buf, sizeof(buf) - 1, 0);
    if (recvResult > 0) {
        // Successfully received data
        buf[recvResult] = '\0';  // Add null terminator so buf is a valid C string
        std::cout << "Server says: " << buf << "\n";
    } else if (recvResult == 0) {
        // Server closed the connection gracefully (0 bytes received = connection closed)
        std::cout << "Connection closed\n";
    } else {
        // recv() returned SOCKET_ERROR (negative value)
        #ifdef _WIN32
            std::cerr << "recv failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "recv failed with error: " << strerror(errno) << "\n";
        #endif
    }

    // Step 4: Get user input and send it to the server
    // ================================================
    std::string name;
    // getline() reads a full line from standard input (including spaces) and stores in 'name'
    std::getline(std::cin, name);
    // send() transmits data to the server over the sockfd socket
    // name.c_str() converts the string to a C-style char array
    // name.size() is the number of bytes to send
    // The last argument (0) means no special flags
    // Returns number of bytes actually sent, or SOCKET_ERROR on failure
    if (send(sockfd, name.c_str(), (int)name.size(), 0) == SOCKET_ERROR) {
        #ifdef _WIN32
            std::cerr << "send failed with error: " << WSAGetLastError() << "\n";
        #else
            std::cerr << "send failed with error: " << strerror(errno) << "\n";
        #endif
    }

    // Step 5: Clean up and close the socket
    // =====================================
    closesocket(sockfd);   // Close the socket connected to the server
    #ifdef _WIN32
        WSACleanup();          // Shut down Winsock and release resources (Windows only)
    #endif
    return 0;  // Return 0 to OS indicating successful execution
}