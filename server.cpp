#include "network\csapp.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <cstring>
#include <vector>

#include "game\game.h"

#define PORT 8080

using namespace std;

int num_players = 0;

atomic<int> ready_count(0);     // how many players pressed 'y'
atomic<int> connected_count(0); // how many players have connected

vector<socket_t> client_socks;
mutex socks_mutex;

// per-client thread
void handle_client(socket_t connfd, int player_id, Game* game) {
    rio_t rio;
    rio_readinitb(&rio, connfd);

    char buf[256];

    // --- Greet the player ---
    string greet = "\n=== Phase 10 ===\n"
                        "Welcome, Player " + to_string(player_id + 1) + "!\n"
                        "Waiting for all players to connect...\n";
    rio_writen(connfd, greet.c_str(), greet.size());

    // Wait until all players are connected before asking to ready up
    while (connected_count < num_players) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    string prompt = "All players connected!\n"
                    "Press 'y' then Enter to ready up: \n";
    rio_writen(connfd, prompt.c_str(), prompt.size());

    // --- Read until the player presses 'y' ---
    while (true) {
        memset(buf, 0, sizeof(buf));
        ssize_t n = rio_readlineb(&rio, buf, sizeof(buf));
        if (n <= 0) {
            cout << "[server] Player " << (player_id + 1) << " disconnected.\n";
            return;
        }

        // Strip trailing newline / carriage return
        for (int i = 0; buf[i]; i++) {
            if (buf[i] == '\r' || buf[i] == '\n') { buf[i] = '\0'; break; }
        }

        if (buf[0] == 'y' || buf[0] == 'Y') {
            ready_count++;
            cout << "[server] Player " << (player_id + 1)
                 << " is ready! (" << ready_count << "/" << num_players << ")\n";

            string ack = "You are ready! Waiting for other players...\n";
            rio_writen(connfd, ack.c_str(), ack.size());
            break;
        } else {
            string retry = "Not recognised. Press 'y' then Enter: ";
            rio_writen(connfd, retry.c_str(), retry.size());
        }
    }

    // --- Spin until all players are ready ---
    while (ready_count < num_players) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // --- Notify this player ---
    string start_msg = "\nGame is starting!! Good Luck!\n";
    rio_writen(connfd, start_msg.c_str(), start_msg.size());

    // --- Game loop ---
    game->player_loop(player_id, connfd);

    // Clean up
    close_socket(connfd);
    cout << "[server] Closed connection to Player " << (player_id + 1) << "\n";
}

// ── main ───────────────────────────────────────────────────────────────────
int main() {
    init_winsock();

    // Ask the server host how many players
    while (true) {
        cout << "How many players? (2-6): ";
        cin >> num_players;
        if (num_players >= 2 && num_players <= 6) break;
        cout << "Invalid. Enter a number between 2 and 6.\n";
    }

    socket_t listenfd = open_listenfd(PORT);
    if (listenfd == INVALID_SOCK) {
        cerr << "Failed to open listen socket.\n";
        cleanup_winsock();
        return 1;
    }

    cout << "[server] Waiting for " << num_players << " players...\n";

    client_socks.resize(num_players);

    // --- Accept ALL connections first, before spawning any threads ---
    for (int i = 0; i < num_players; i++) {
        struct sockaddr_in clientaddr;
        socklen_t clientlen = sizeof(clientaddr);

        socket_t connfd = accept(listenfd,
                                 (struct sockaddr *)&clientaddr,
                                 &clientlen);
        if (connfd == INVALID_SOCK) {
            cerr << "[server] accept() failed\n";
            continue;
        }

        {
            lock_guard<mutex> lock(socks_mutex);
            client_socks[i] = connfd;
        }

        connected_count++;
        cout << "[server] Player " << (i + 1) << " connected. ("
             << connected_count << "/" << num_players << ")\n";
    }

    // --- Create the game now that all sockets are known ---
    Game* game = new Game(num_players, client_socks);

    // --- Spawn one thread per player, passing the shared game ---
    vector<thread> threads(num_players);
    for (int i = 0; i < num_players; i++) {
        threads[i] = thread(handle_client, client_socks[i], i, game);
    }

    // --- Wait for all threads to finish ---
    for (int i = 0; i < num_players; i++) {
        if (threads[i].joinable()) threads[i].join();
    }

    delete game;
    close_socket(listenfd);
    cleanup_winsock();

    cout << "[server] All done. Shutting down.\n";
    return 0;
}