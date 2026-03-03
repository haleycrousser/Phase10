#include "deck.h"
#include "../Card/card.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

Deck::Deck() {
        string colors[4] = {"Red", "Green", "Yellow", "Blue"};
        int numbers[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
        char variants[2] = {'a','b'};

        standardDeck.reserve(96 + 4 + 8);  

        
        for (const auto& color : colors) {
            for (int num : numbers) {
                for (char var : variants) {
                    int pts = (num < 10) ? 5 : 10;
                    standardDeck.push_back(Card(color, num, var, pts));
                }
            }
        }

        
        for (const auto& color : colors) {
            standardDeck.push_back(Card(color, 0, 's', 15));
        }

        
        for (int i = 0; i < 8; ++i) {
            standardDeck.push_back(Card("", 0, 'w', 25)); 
        }
    }
