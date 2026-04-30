#ifndef DECK_H
#define DECK_H
 
#include "card.h"
#include <vector>
 
class Deck {
public:
    Deck();
 
    void    shuffleDeck();
    Card    drawCard();       // pop from top
    bool    isEmpty() const;
    int     size()    const;
 
    // debug
    void printDeck();
 
private:
    std::vector<Card> cards;  // index 0 = bottom, back() = top
    void buildDeck();
};
 
#endif
 