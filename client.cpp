#include "network\csapp.h" //using my csapp wrapper for sockets and rio functions
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>

#define HOST "127.0.0.1" //server address, using localhost for testing on the same computer
#define PORT 8080        //server port the client connects to

using namespace std;

atomic<bool> running(true); //keeps the receive loop running while the client is connected

void receive_loop(socket_t fd) { //keeps receiving messages from the server while the user can still type
    char buf[1024]; //stores messages coming from the server

    while (running) {
        memset(buf, 0, sizeof(buf)); //clear the buffer before receiving a new message

#ifdef _WIN32
        int n = recv(fd, buf, sizeof(buf) - 1, 0); //windows recv returns int
#else
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0); //linux/mac recv returns ssize_t
#endif

        if (n <= 0) { //server closed connection or something went wrong
            this_thread::sleep_for(chrono::milliseconds(500));
            cout << "\n[client] Server disconnected.\n";
            exit(0);
        }

        buf[n] = '\0'; //make sure the received message ends like a string

        this_thread::sleep_for(chrono::milliseconds(500));
        cout << buf; //print whatever the server sent
        cout.flush();
    }
}

int main() {
    init_winsock(); //starts winsock on windows before using sockets

    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[client] Connecting ...\n";

    socket_t fd = open_clientfd(HOST, PORT); //connect to the server
    if (fd == INVALID_SOCK) {
        this_thread::sleep_for(chrono::milliseconds(500));
        cerr << "[client] Could not connect to server.\n";
        cleanup_winsock();
        return 1;
    }

    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[client] Connected!\n";

    thread recv_thread(receive_loop, fd); //separate thread so server messages can print while user types

    string line;
    while (running && getline(cin, line)) { //read user input and send it to the server
        line += '\n'; //server expects lines ending with newline

        if (rio_writen(fd, line.c_str(), line.size()) < 0) {
            this_thread::sleep_for(chrono::milliseconds(500));
            cerr << "[client] Send failed.\n";
            break;
        }
    }

    running = false; //tell receive loop to stop
    close_socket(fd); //close connection to server

    if (recv_thread.joinable()) recv_thread.join(); //wait for receive thread to finish

    cleanup_winsock(); //cleanup winsock before ending program
    return 0;
}