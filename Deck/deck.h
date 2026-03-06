#ifndef DECK_H
#define DECK_H
#include "../Card/card.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>


class Deck {

    public:
    
    std::vector<Card> standardDeck;
    std::vector<Card> discardPile;

    Deck(); //constructor

    void printStandardDeck() const; //print standardDeck

    void shuffleDeck();

};

#endif