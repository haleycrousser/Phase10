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
    std::vector<Card> shuffledDeck;
    std::vector<Card> discardPile;

    Deck() {
        std::string colors[4] = {"Red", "Green", "Yellow", "Blue"};
        int numbers[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
        char variants[2] = {'a','b'};

        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 12; ++j)
                for (int k = 0; k < 2; ++k)
                    standardDeck.push_back(Card(colors[i], numbers[j], variants[k]));
    }


    void shuffleCards() {
        std::random_device rd; std::mt19937 gen(rd()); //random number generator
        shuffle(standardDeck.begin(), standardDeck.end(), gen);
        return;
    }

};

#endif