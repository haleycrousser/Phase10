#ifndef DECK_H
#define DECK_H

#include "card.h"
#include <stack>

class Deck {

public:

    std::stack<Card> standardDeck;
    std::stack<Card> mainDeck;

    
    Deck();
    void shuffleDeck();

//debug functions
    void printStandardDeck();
    void printDiscardPile();
};

#endif