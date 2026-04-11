// client.cpp
// Compile: g++ -std=c++17 client.cpp csapp/csapp.cpp -o client -lws2_32
 
#include "csapp/csapp.h"
 
int main() {
 
    ////////// CONNECT TO SERVER //////////
    SOCKET sockfd = connectToServer("127.0.0.1", "8080");
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }
 
    ////////// RECEIVE QUESTION FROM SERVER //////////
    char buf[256] = {};
    int n = recv(sockfd, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "Server says: " << buf << "\n";
    } else if (n == 0) {
        std::cout << "Connection closed\n";
        return 1;
    } else {
        NET_ERR("recv failed");
        return 1;
    }
 
    ////////// GET USER INPUT AND SEND TO SERVER //////////
    std::string name;
    std::getline(std::cin, name);
    if (send(sockfd, name.c_str(), (int)name.size(), 0) == SOCKET_ERROR) {
        NET_ERR("send failed");
    }
 
    ////////// CLEAN UP //////////
    closeConnection(sockfd);
    return 0;
}