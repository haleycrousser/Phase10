#ifndef GAME_H
#define GAME_H

#include "../network/csapp.h"
#include "card.h"
#include "deck.h"
#include "player.h"

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Game {
public:
    Game(int num_players, std::vector<socket_t>& sockets);

    // Each player thread calls this — blocks until game over
    void player_loop(int player_id, socket_t fd);

private:
    int num_players;
    std::vector<socket_t> socks;
    std::vector<rio_t>    rios;
    std::vector<Player>   players;
    Deck              deck;
    std::vector<Card> discardPile;  // back() = top card

    std::atomic<int>  current_player{0};
    std::atomic<bool> game_over{false};
    std::mutex              turn_mutex;
    std::condition_variable turn_cv;

    // ── Core loop ──────────────────────────────────────────────────────────
    void run();

    // ── Turn actions ───────────────────────────────────────────────────────
    void dealCards();                   // deal 10 cards each, flip discard
    void sendHand(int player_id);       // send formatted hand to one player
    void drawPhase(int player_id);      // draw from deck or discard pile
    void layDownPhase(int player_id);   // select cards, validate, lay down
    void hitPhase(int player_id);       // add a card to any phase area
    void discardCard(int player_id);    // must discard one card per turn

    // ── Round end ──────────────────────────────────────────────────────────
    void tallyScores();     // add handScore() penalty to each player
    bool checkGameWon();    // true if anyone has passed phase 10

    // ── Helpers ────────────────────────────────────────────────────────────
    void        broadcast(const std::string& msg);
    void        send_to(int player_id, const std::string& msg);
    std::string recv_from(int player_id);   // blocking, strips \r\n

    std::string          phaseDescription(int phaseNum);
    std::string          cardToString(const Card& c);
    std::vector<int>     parseIndices(const std::string& input, int handSize);
};

#endif