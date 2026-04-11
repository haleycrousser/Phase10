#include "csapp/csapp.h"

using namespace std;
 
int main() {
 
    ////////// CONNECT TO SERVER //////////
    SOCKET sockfd = connectToServer("127.0.0.1", "8080");
 
    ////////// WOULD YOU LIKE TO START THE GAME? //////////
    char buf[256] = {};

    while (true) {

        memset(buf, 0, sizeof(buf));
        int n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        buf[n] = '\0';
        cout << "Server says: " << buf;

        string ans;
        getline(cin, ans);
        if (send(sockfd, ans.c_str(), (int)ans.size(), 0) == SOCKET_ERROR) NET_ERR("send failed");

        memset(buf, 0, sizeof(buf));
        n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        buf[n] = '\0';
        cout << "Server says: " << buf << "\n";

        if (strcmp(buf, "Starting the game!") == 0) break;
    }
/////////---------------------------------------------------------------------------/////////

    ///////// GAME STARTS HERE //////////
    memset(buf, 0, sizeof(buf)); int n = recv(sockfd, buf, sizeof(buf) - 1, 0); buf[n] = '\0';
    cout << "Server says: " << buf;

    int players;
    cin >> players;
    if (send(sockfd, to_string(players).c_str(), (int)to_string(players).size(), 0) == SOCKET_ERROR) NET_ERR("send failed");


/////////---------------------------------------------------------------------------/////////
    ////////// CLEAN UP //////////
    closeConnection(sockfd);
    return 0;
}