#include "network\csapp.h" //using my csapp wrapper for sockets and rio functions
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <cstring>
#include <vector>

#include "game\game.h" //game class that runs the actual phase 10 game logic

#define PORT 8080 //port the server listens on

using namespace std;

int num_players = 0; //number of players the server is waiting for

atomic<int> ready_count(0);     //counts how many players pressed y
atomic<int> connected_count(0); //counts how many players connected to the server

vector<socket_t> client_socks; //stores all connected player sockets
mutex socks_mutex;             //protects client_socks when adding sockets

void handle_client(socket_t connfd, int player_id, Game* game) { //handles one connected player
    rio_t rio; //rio buffer for reading from this client
    rio_readinitb(&rio, connfd);

    char buf[256]; //stores input from the player

    string greet = "\n=== Phase 10 ===\n"
                        "Welcome, Player " + to_string(player_id + 1) + "!\n"
                        "Waiting for all players to connect...\n";
    rio_writen(connfd, greet.c_str(), greet.size());

    while (connected_count < num_players) { //wait until every player has connected
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    string prompt = "All players connected!\n"
                    "Press 'y' then Enter to ready up: \n";
    rio_writen(connfd, prompt.c_str(), prompt.size());

    while (true) { //keeps asking until player types y
        memset(buf, 0, sizeof(buf));
        ssize_t n = rio_readlineb(&rio, buf, sizeof(buf));
        if (n <= 0) {
            this_thread::sleep_for(chrono::milliseconds(500));
            cout << "[server] Player " << (player_id + 1) << " disconnected.\n";
            return;
        }

        //remove newline so the input can be compared cleanly
        for (int i = 0; buf[i]; i++) {
            if (buf[i] == '\r' || buf[i] == '\n') { buf[i] = '\0'; break; }
        }

        if (buf[0] == 'y' || buf[0] == 'Y') {
            ready_count++;
            this_thread::sleep_for(chrono::milliseconds(500));
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

    while (ready_count < num_players) { //wait until all players are ready
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    string start_msg = "\nGame is starting!! Good Luck!\n";
    rio_writen(connfd, start_msg.c_str(), start_msg.size());

    game->player_loop(player_id, connfd); //send this player into the game loop

    close_socket(connfd); //close this client's socket once the game is finished
    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[server] Closed connection to Player " << (player_id + 1) << "\n";
}

int main() {
    init_winsock(); //starts winsock on windows before using sockets

    while (true) { //ask host how many players should join
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "How many players? (2-6): ";
        cin >> num_players;
        if (num_players >= 2 && num_players <= 6) break;
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "Invalid. Enter a number between 2 and 6.\n";
    }

    socket_t listenfd = open_listenfd(PORT); //create the listening socket
    if (listenfd == INVALID_SOCK) {
        this_thread::sleep_for(chrono::milliseconds(500));
        cerr << "Failed to open listen socket.\n";
        cleanup_winsock();
        return 1;
    }

    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[server] Waiting for " << num_players << " players...\n";

    client_socks.resize(num_players); //make room for all player sockets

    for (int i = 0; i < num_players; i++) { //accept players until the game is full
        struct sockaddr_in clientaddr;
        socklen_t clientlen = sizeof(clientaddr);

        socket_t connfd = accept(listenfd,
                                 (struct sockaddr *)&clientaddr,
                                 &clientlen);
        if (connfd == INVALID_SOCK) {
            this_thread::sleep_for(chrono::milliseconds(500));
            cerr << "[server] accept() failed\n";
            continue;
        }

        {
            lock_guard<mutex> lock(socks_mutex); //lock while saving the socket
            client_socks[i] = connfd;
        }

        connected_count++;
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "[server] Player " << (i + 1) << " connected. ("
             << connected_count << "/" << num_players << ")\n";
    }

    Game* game = new Game(num_players, client_socks); //make the game after all sockets are connected

    vector<thread> threads(num_players); //one thread for each player
    for (int i = 0; i < num_players; i++) {
        threads[i] = thread(handle_client, client_socks[i], i, game);
    }

    for (int i = 0; i < num_players; i++) { //wait for every player thread to finish
        if (threads[i].joinable()) threads[i].join();
    }

    delete game; //free game memory
    close_socket(listenfd); //close server socket
    cleanup_winsock(); //cleanup winsock before ending program

    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[server] All done. Shutting down.\n";
    return 0;
}