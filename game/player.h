#ifndef PLAYER_H
#define PLAYER_H

#include "card.h"
#include <string>
#include <vector>

class Player {
public:
    int               player_id;
    std::string       name;
    std::vector<Card> hand;
    std::vector<Card> phaseArea;   // cards laid down to complete phase
    int               currentPhase;
    int               score;
    bool              completedPhase;

    Player();
    Player(int id, std::string name);

    void addCard(Card c);           // draw a card into hand
    void removeCard(int index);     // discard by index
    int  handScore() const;         // sum of point values still in hand
    void printHand() const;         // debug
};

#endif