#ifndef DECK_H
#define DECK_H

#include "card.h"
#include <stack>

class Deck {
    std::stack<Card> standardDeck;
    std::stack<Card> discardPile;
public:
    Deck();
    void shuffleDeck();

//debug functions
    void printStandardDeck();
    void printDiscardPile();
};

#endif