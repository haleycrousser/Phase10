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
    Game(int num_players, std::vector<socket_t>& sockets); //constructor, takes number of players and the socket list from the server
    void player_loop(int player_id, socket_t fd); //starts the game loop for player 0 and keeps other player threads waiting until game ends

private:
    int num_players;                 //number of players in the game
    std::vector<socket_t> socks;      //stores each player's socket so the game can send messages to them
    std::vector<rio_t>    rios;       //stores rio buffers for reading player input
    std::vector<Player>   players;    //stores all player objects
    Deck              deck;           //main draw deck
    std::vector<Card> discardPile;    //discard pile cards

    std::atomic<int>  current_player{0};   //keeps track of whose turn it is
    std::atomic<bool> game_over{false};    //used to tell all threads when the game is over
    std::mutex              turn_mutex;    //mutex used with the condition variable
    std::condition_variable turn_cv;       //wakes up waiting player threads when the game ends

    void run(); //main game loop, handles rounds and player turns

  
    void dealCards();              //deals cards to each player and resets round info
    void sortHand(int player_id);  //sorts a player's hand so it is easier to read
    void sendHand(int player_id);  //sends the player's current hand and phase info
    void drawPhase(int player_id); //handles drawing from deck or discard pile
    void layDownPhase(int player_id); //lets player choose cards to lay down their phase
    void hitPhase(int player_id);     //hits valid cards onto completed phase areas
    void discardCard(int player_id);  //lets player discard one card at the end of their turn


    void tallyScores();     //adds leftover hand points to each player's score
    bool checkGameWon();    //checks if someone passed phase 10 and announces winner


    bool        cardFitsArea(const Card& card, const std::vector<Card>& area, int phaseNum); //checks if a card can be hit onto a phase area
    void        broadcast(const std::string& msg); //sends a message to every player
    void        send_to(int player_id, const std::string& msg); //sends a message to one specific player
    std::string recv_from(int player_id); //reads input from one specific player
    void        pause(int ms = 1000); //small delay so messages do not print too fast

    std::string      formatPhaseArea(const std::vector<Card>& area, int phaseNum); //formats a player's laid down phase area
    std::string      phaseAreasString(); //builds the string showing all phase areas
    std::string      phaseDescription(int phaseNum); //returns the rule for a phase number
    std::string      cardToString(const Card& c); //converts a card into readable text
    std::vector<int> parseIndices(const std::string& input, int handSize); //turns typed card numbers into valid hand indexes
};

#endif