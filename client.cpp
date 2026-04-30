#include "network\csapp.h"
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <atomic>

#define HOST "127.0.0.1"
#define PORT 8080

using namespace std;

// ── Receiver thread ────────────────────────────────────────────────────────
// Continuously reads lines from the server and prints them.
// Runs concurrently so the main thread can read user input at the same time.
std::atomic<bool> running(true);

void receive_loop(socket_t fd) {
    rio_t rio;
    rio_readinitb(&rio, fd);

    char buf[1024];
    while (running) {
        memset(buf, 0, sizeof(buf));
        ssize_t n = rio_readlineb(&rio, buf, sizeof(buf));
        if (n <= 0) {
            // Server closed connection — exit immediately so the user
            // doesn't have to press Enter again to unblock getline
            std::cout << "\n[client] Server disconnected.\n";
            exit(0);
        }
        // Print whatever the server sends, flushing immediately
        std::cout << buf;
        std::cout.flush();
    }
}

// ── main ───────────────────────────────────────────────────────────────────
int main() {
    init_winsock();

    std::cout << "[client] Connecting ...\n";

    socket_t fd = open_clientfd(HOST, PORT);
    if (fd == INVALID_SOCK) {
        std::cerr << "[client] Could not connect to server.\n";
        cleanup_winsock();
        return 1;
    }

    std::cout << "[client] Connected!\n";

    // Start background thread that prints server messages
    std::thread recv_thread(receive_loop, fd);

    // Main thread: read input from this terminal and send it to the server
    std::string line;
    while (running && std::getline(std::cin, line)) {
        line += '\n';  // server reads until '\n'
        if (rio_writen(fd, line.c_str(), line.size()) < 0) {
            std::cerr << "[client] Send failed.\n";
            break;
        }
    }

    running = false;
    close_socket(fd);

    if (recv_thread.joinable()) recv_thread.join();

    cleanup_winsock();
    return 0;
}