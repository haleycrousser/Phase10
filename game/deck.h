#ifndef DECK_H
#define DECK_H
 
#include "card.h"
#include <vector>
 
class Deck {
public:
    Deck();
 
    void    shuffleDeck();
    Card    drawCard();       // pop from top
    Card    peekCard() const; // view top card without removing it
    bool    isEmpty() const;
    int     size()    const;
 
    void printDeck(); // debug
 
private:
    std::vector<Card> cards;  // index 0 = bottom, back() = top
    void buildDeck();
};
 
#endif