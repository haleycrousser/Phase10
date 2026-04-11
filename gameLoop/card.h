#ifndef CARD_H
#define CARD_H

#include <iostream>

class Card {
    
public:
    string color;
    int    num;
    char   variant;
    int    points;
    bool   wild;
    bool   skip;

    Card(string color, int num, char variant, int points, bool wild, bool skip);
    static Card createCard(string color, int num, char variant, int points);
};

#endif