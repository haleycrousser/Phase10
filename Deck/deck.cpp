#include "deck.h"
#include "../Card/card.h"

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>


using namespace std;

Deck::Deck() {
        string colors[4] = {"Red", "Green", "Yellow", "Blue"}; //4 colors
        int numbers[12] = {1,2,3,4,5,6,7,8,9,10,11,12}; //numbers
        char variants[2] = {'a','b'}; //variant

        standardDeck.reserve(96 + 4 + 8);  //reserves space in vector -> 108 cards total

        
        for (const auto& color : colors) { //for each color
            for (int num : numbers) { //for each number
                for (char var : variants) { //for each variant
                    int pts = (num < 10) ? 5 : 10; //if num less then 10 -> 5 else 10 points
                    standardDeck.push_back(Card(color, num, var, pts)); //create card object and adding it to the deck
                }
            }
        }

        for (int i = 0; i < 4; ++i) {
            standardDeck.push_back(Card("Skip", 0, 's', 15)); //skip card worth 15 points
        }

        
        for (int i = 0; i < 8; ++i) {
            standardDeck.push_back(Card("Wild", 0, 'w', 25)); //wild card worth 25 points
        }
    }


    void Deck::printStandardDeck() const {
    for (const auto& card : standardDeck) {
        std::cout << card << std::endl; // its printing the object, but without overloading the operator ir doesnt know what data to actually print
    }
}


void Deck::shuffleDeck() {

    random_device rd;
    mt19937 gen(rd());

    shuffle(standardDeck.begin(), standardDeck.end(), gen);


    return;

}