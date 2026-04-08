#ifndef PLAYER_H
#define PLAYER_H

#include "card.h"
#include <string>
#include <vector>

class Player { 

    std::string name; 
    std::vector<Card> hand; 
    int currentPhase;
    int score; 
    bool isAI; 

};

#endif