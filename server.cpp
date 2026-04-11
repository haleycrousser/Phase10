#include "csapp/csapp.h"
#include "gameLoop/card.h"

#include <cstring>
 
int main() {
 
    ////////// CREATE SERVER //////////
    SOCKET serverfd = createServer("8080");
 
    ////////// WAIT FOR CLIENT //////////
    SOCKET clientfd = listenForClient(serverfd);
 
    ////////// GAME STUFF //////////
    const char *question = "What is your name? ";
    if (send(clientfd, question, (int)strlen(question), 0) == SOCKET_ERROR) {
        NET_ERR("send failed");
    }
 
    char buf[256] = {};
    int n = recv(clientfd, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "Client's name: " << buf << "\n";
    } else if (n == 0) {
        std::cout << "Connection closed\n";
    } else {
        NET_ERR("recv failed");
    }
 
    ////////// CLEAN UP //////////
    closeServer(clientfd, serverfd);
    return 0;
}
 