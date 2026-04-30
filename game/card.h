#ifndef CARD_H
#define CARD_H

#include <string>
#include <iostream>

class Card {

public:
//card properties
    std::string color;
    int         num;
    char        variant;
    int         points;
    bool        wild;
    bool        skip;

    Card(std::string color, int num, char variant, int points, bool wild, bool skip); //card constructor (declare card obj)
    static Card createCard(std::string color, int num, char variant, int points); //create card obj
};

// Stream operator declared here so anything including card.h can use it
std::ostream& operator<<(std::ostream& os, const Card& card);

#endif