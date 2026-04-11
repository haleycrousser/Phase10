#ifndef CARD_H
#define CARD_H

#include <iostream>
#include <string>

class Card {

public:

    std::string color;
    int    num;
    char   variant;
    int    points;
    bool   wild;
    bool   skip;

    Card(std::string color, int num, char variant, int points, bool wild, bool skip);
    static Card createCard(std::string color, int num, char variant, int points);
};

#endif